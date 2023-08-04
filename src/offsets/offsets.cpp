#include <dlfcn.h>
#include <marl/defer.h>
#include <marl/scheduler.h>
#include <marl/waitgroup.h>
#include <algorithm>
#include <bit>
#include <cstdint>
#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/view/chunk.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/sliding.hpp>
#include <range/v3/view/zip.hpp>
#include <tracy/Tracy.hpp>

// It's just a header, it can't hurt you
#include "util/loaded_module.hpp"
// (Must be AFTER loaded_module)
#include <link.h>

#include "offsets/eh_frame.hpp"
#include "offsets/offsets.hpp"
#include "offsets/rtti.hpp"
#include "util/data_range_checker.hpp"
#include "util/error.hpp"
#include "util/hexdump.hpp"
#include "util/timedscope.hpp"

std::forward_list<Offset*> g_offset_list{};

uintptr_t SharedLibOffset::get_address(ModuleRangeMap& modules,
                                       ModuleVtables& vtables,
                                       EhFrameRanges& eh_frame) const {
  auto base_address = modules.at(module).begin;

  if (base_address == 0)
    throw StringError("Failed to get address of module: {}", module);
  return base_address + offset;
}

uintptr_t VtableOffset::get_address(ModuleRangeMap& modules,
                                    ModuleVtables& vtables,
                                    EhFrameRanges& eh_frame) const {
  auto vftable_ptr =
      vtables.at(module).at(std::to_string(name.size()) + name).at(vftable);
  return *std::bit_cast<uintptr_t*>(vftable_ptr + (sizeof(void*) * function));
}

uintptr_t SharedLibSymbol::get_address(ModuleRangeMap& modules,
                                       ModuleVtables& vtables,
                                       EhFrameRanges& eh_frame) const {
  auto handle = dlopen(module.c_str(), RTLD_LAZY | RTLD_NOLOAD);
  if (handle == nullptr) {
    throw StringError("Failed to get handle for module {}", module);
  }

  auto addr = std::bit_cast<uintptr_t>(dlsym(handle, symbol.c_str()));
  if (addr == 0)
    throw StringError("Failed to get address of symbol {} from module {}\n",
                      module, symbol);
  return addr;
}

uintptr_t SharedLibSignature::get_address(ModuleRangeMap& modules,
                                          ModuleVtables& vtables,
                                          EhFrameRanges& eh_frame) const {
  auto module = modules.at(module_name);
  auto module_memory_span = module.data_at_mem();

  // hexdump(signature.data(), signature.size());
  // hexdump(mask.data(), mask.size());

  auto window = ranges::views::sliding(module_memory_span, signature.size());
  for (const auto& [idx, bytes] : window | ranges::views::enumerate) {
    if (ranges::all_of(ranges::views::zip(bytes, signature, mask), [](auto t) {
          auto [orig_byte, sig_byte, mask_byte] = t;
          return (orig_byte & mask_byte) == (sig_byte & mask_byte);
        })) {
      return idx + module.begin;
    }
  }

  throw StringError("Couldn't find pattern in memory range");
}

void init_offsets() {
  TimedScope("offsets");
  ModuleVtables module_vtables{};
  EhFrameRanges eh_frame_ranges{};
  ModuleRangeMap modules{};

  dl_iterate_phdr(
      [](dl_phdr_info* info, size_t info_size, void* user_data) {
        ZoneScopedN("dl_iterate_phdr func");

        auto* modules = std::bit_cast<ModuleRangeMap*>(user_data);
        const std::string_view fname = basename(info->dlpi_name);

        constexpr std::string_view excluded_paths[] = {"linux-vdso.so.1",
                                                       "libclientplugin.so"};

        if (info->dlpi_addr == 0 || info->dlpi_name == nullptr ||
            info->dlpi_name[0] == '\0') {
          return 0;
        } else if (ranges::any_of(excluded_paths,
                                  [&](auto b) { return fname == b; })) {
          return 0;
        }

        auto end_addr = info->dlpi_addr;
        for (size_t i = 0; i < info->dlpi_phnum; i++) {
          end_addr = info->dlpi_addr +
                     (info->dlpi_phdr->p_vaddr + info->dlpi_phdr->p_memsz);
        }

        modules->insert(
            {std::string(info->dlpi_name),
             DataRange(info->dlpi_addr, end_addr - info->dlpi_addr)});

        return 0;
      },
      &modules);

  marl::Scheduler sched(marl::Scheduler::Config().setWorkerThreadCount(4));
  sched.bind();
  defer(sched.unbind());

  ModuleRangeMap cleaned_modules;

  marl::WaitGroup wg(modules.size());
  for (auto& [mod_name, mod_range] : modules) {
    const std::string fname = basename(mod_name.c_str());
    cleaned_modules.insert({fname, mod_range});

    std::vector<DataRange>& mod_eh_frame_ranges = eh_frame_ranges[fname];
    Vtables& vtables = module_vtables[fname];

    marl::schedule([&, fname, wg]() {
      // TimedScope(fmt::format("module {}", mod_name));
      ZoneScopedN("handle module");
      defer(wg.done());

      try {
        LoadedModule loaded_mod{
            mod_name,
            mod_range,
        };

        for (auto& r : get_eh_frame_ranges(loaded_mod)) {
          mod_eh_frame_ranges.push_back(r);
        }

        for (auto& vtable :
             get_vtables_from_module(loaded_mod, mod_eh_frame_ranges)) {
          ZoneScopedN("vtable insertion");
          vtables.insert(vtable);
        }
      } catch (std::exception& e) {
        fmt::print(
            "while handling module {}:\n"
            "\t{}\n",
            fname, e.what());
        throw;
      };
    });
  }

  wg.wait();

  for (auto* offset : g_offset_list) {
    offset->cache_address(cleaned_modules, module_vtables, eh_frame_ranges);
  }
};

namespace offsets {
  const SharedLibSymbol SDL_GL_SwapWindow{"libSDL2-2.0.so.0",
                                          "SDL_GL_SwapWindow"};

  // FID HASH F: f2bcda30797732a8, X: b724bdb369b5da37
  const SharedLibSignature FindAndHealTargets{
      "client.so",
      "5589e557565381ec5c01000065a1140000008945e431c08b5d088b0da423fa018b83f4"
      "04"
      "000085c0742283f8ff0fb7d0bfff1f00000f44d7c1e20401ca89d183c1047408c1e810"
      "39"
      "4104742431ff8b5de465331d1400000089f80f8564040000",
      "ffffffffffffffff00000000ffff00000000fff800fffffff800ffff00000000fff800"
      "00"
      "0000ffffff00ffff00ffffffff00000000ffffffffff00ffffffffffff00ff00ffff00"
      "ff"
      "f800ff00fffffff800ffffff00000000ffffffff00000000"};
  // FH: 129c0557c56d8e10 (81) +53 XH: 38473da5967e6f22
  const SharedLibSignature CNavMesh_GetNavDataFromFile{
      "server.so",
      "5589e557568dbde0feffff5381ec3c02000065a1140000008945e431c0a1306989018b"
      "5d"
      "0c8b75108b503cb89aeb410185d20f45c28d95e0fdffff8904248954",
      "f8ffc0f8f8ffc000000000f8fff800000000ffff00000000ffc000ffc0ff00000000ff"
      "c0"
      "00ffc000ffc000f800000000ffc0ffffc0ffc000000000ffc738ffc7"};

  const SharedLibSignature GetPlayerNameForSteamID{
      "client.so",
      "5589e55383ec14a1c823fa018b48088b45108b198b50048b00890c248954240889442404"
      "ff531c8b550cc744240c04000000890424895424088b550889542404e80bac100083c414"
      "5b5dc3",
      "ffffffffffff00ff00000000fff800fff800fff8fff800fff8ffff38ffff3800ffff3800"
      "fff800fff800ffff380000000000ffff38ffff3800fff800ffff3800ff00000000ffff00"
      "ffffff"};

  const SharedLibSymbol g_Telemetry{"libtier0.so", "g_Telemetry"};
  const SharedLibSymbol TelemetryTick{"libtier0.so", "TelemetryTick"};
  const SharedLibSymbol ThreadSetDebugName{"libtier0.so", "ThreadSetDebugName"};
  const SharedLibSymbol CVProfNode_EnterScope{"libtier0.so",
                                              "_ZN10CVProfNode10EnterScopeEv"};
  const SharedLibSymbol CVProfNode_ExitScope{"libtier0.so",
                                             "_ZN10CVProfNode9ExitScopeEv"};
  const VtableOffset CEngine_Frame{"engine.so", "CEngine", 6};

  const VtableOffset CTFCleaver_CreateJarProjectile{"server.so", "CTFCleaver",
                                                    504};
  const VtableOffset CGameServer_FinishCertificateCheck{"engine.so",
                                                        "CGameServer", 55};

  const SharedLibOffset NotificationQueue_Add{"client.so",
                                              0x00f8f890 - 0x10000};

  const SharedLibOffset CEconNotification_CEconNotification{
      "client.so", 0x00f8ec70 - 0x10000};

  const extern SharedLibOffset CEconNotification_SetKeyValues{
      "client.so", 0x00f8ed80 - 0x10000};
}  // namespace offsets
