#include <dlfcn.h>
#include <marl/defer.h>
#include <marl/scheduler.h>
#include <marl/waitgroup.h>
#include <algorithm>
#include <bit>
#include <cstdint>
#include <elfio/elfio.hpp>
#include <mutex>
#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/view/chunk.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/sliding.hpp>
#include <range/v3/view/zip.hpp>
#include <tracy/Tracy.hpp>

// It's just a header, it can't hurt you
// (Must be AFTER elfio)
#include <link.h>

#include "offsets/eh_frame.hpp"
#include "offsets/offsets.hpp"
#include "offsets/rtti.hpp"
#include "util/error.hpp"
#include "util/timedscope.hpp"

std::forward_list<Offset*> g_offset_list{};

LoadedModule::LoadedModule(const std::string& path,
                           const uintptr_t base_address)
    : base_address(base_address) {
  ZoneScoped;
  elf.load(path);

  for (auto& segment : elf.segments) {
    if (segment->get_type() == /* ELFIO:: */ PT_LOAD) {
      offline_baseaddr =
          std::min(offline_baseaddr,
                   static_cast<uintptr_t>(segment->get_virtual_address()));
    }
  }

  if (base_address == 0) {
    // bail out, this is bad
    throw StringError("WARNING: loaded_mod.base_address is nullptr");
  };
}

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
  auto mod_data = module.data_at_mem();

  fmt::print("SEARCHING {:08X} -> {:08X}\n", module.begin,
             module.begin + module.length);

  auto window = ranges::views::sliding(mod_data, signature.size());
  for (const auto& [idx, bytes] : window | ranges::views::enumerate) {
    // for (auto row : bytes | ranges::views::chunk(16)) {
    //   for (auto b : row)
    //     fmt::print("{:02x} ", b);
    //   fmt::print(" | ");
    //   for (auto b : row) {
    //     auto c = static_cast<char>(b);
    //     if (c != '\n' && isprint(b))
    //       fmt::print("{:c}", b);
    //     else
    //       fmt::print(".");
    //   }
    //   fmt::print("\n");
    // }
    // fmt::print("\n---------------------\n");

    if (ranges::all_of(ranges::views::zip(bytes, signature, mask), [](auto t) {
          auto [orig_byte, sig_byte, mask_byte] = t;
          return (orig_byte & mask_byte) == sig_byte;
        })) {
      fmt::print("FOUND {} {:08x}\n", module_name, idx);
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

  TracyLockable(std::mutex, vtable_mutex);
  marl::WaitGroup wg(modules.size());
  for (auto& [mod_name, mod_range] : modules) {
    marl::schedule([&, wg]() {
      // TimedScope(fmt::format("module {}", mod_name));
      ZoneScopedN("handle module");
      defer(wg.done());

      const std::string fname = basename(mod_name.c_str());

      try {
        LoadedModule loaded_mod{
            mod_name,
            static_cast<uintptr_t>(mod_range.begin),
        };

        std::vector<DataRange> mod_eh_frame_ranges;
        for (auto& r : get_eh_frame_ranges(loaded_mod)) {
          mod_eh_frame_ranges.push_back(r);
        }

        eh_frame_ranges[fname] = mod_eh_frame_ranges;

        for (auto& vtable :
             get_vtables_from_module(loaded_mod, mod_eh_frame_ranges)) {
          ZoneScopedN("vtable insertion");
          std::unique_lock l(vtable_mutex);
          module_vtables[fname].insert(vtable);
        };

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

  ModuleRangeMap cleaned_modules;
  for (auto& [mod_name, mod_range] : modules) {
    const std::string cleaned_name = basename(mod_name.c_str());
    cleaned_modules.insert({cleaned_name, mod_range});
  }

  for (auto* offset : g_offset_list) {
    offset->cache_address(cleaned_modules, module_vtables, eh_frame_ranges);
  }
};

namespace offsets {
  const SharedLibSymbol SDL_GL_SwapWindow{"libSDL2-2.0.so.0",
                                          "SDL_GL_SwapWindow"};

  // FID HASH F: f2bcda30797732a8, X: b724bdb369b5da37
  const SharedLibOffset FindAndHealTargets{"client.so", 0x00dcd6e0};
  const SharedLibOffset CNavMesh_GetNavDataFromFile{"server.so",
                                                    0x00c01d20 - 0x10000};

  const SharedLibSymbol g_Telemetry{"libtier0.so", "g_Telemetry"};
  const SharedLibSymbol TelemetryTick{"libtier0.so", "TelemetryTick"};
  const SharedLibSymbol ThreadSetDebugName{"libtier0.so", "ThreadSetDebugName"};
  const SharedLibSymbol CVProfNode_EnterScope{"libtier0.so",
                                              "_ZN10CVProfNode10EnterScopeEv"};
  const SharedLibSymbol CVProfNode_ExitScope{"libtier0.so",
                                             "_ZN10CVProfNode9ExitScopeEv"};
  const VtableOffset CEngine_Frame{"engine.so", "CEngine", 6};
}  // namespace offsets
