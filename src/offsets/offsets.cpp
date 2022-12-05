#include <dlfcn.h>
#include <frida-gum.h>
#include <cstdint>
#include <elfio/elfio.hpp>
#include <tracy/Tracy.hpp>

#include "offsets/offsets.hpp"
#include "offsets/rtti.hpp"
#include "util/error.hpp"

LoadedModule::LoadedModule(const std::string path,
                           const uintptr_t base_address)
    : base_address(base_address) {
  ZoneScoped;
  elf.load(path);

  for (ELFIO::segment* segment : elf.segments) {
    if (segment->get_type() == ELFIO::PT_LOAD) {
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

uintptr_t SharedLibOffset::get_address() const {
  auto base_address = gum_module_find_base_address(module.c_str());

  if (base_address == 0)
    throw StringError("Failed to get address of module: {}", module);
  return base_address + offset;
}

uintptr_t VtableOffset::get_address() const {
  return g_RTTI->get_function(module, name, vftable, function);
}

uintptr_t SharedLibSymbol::get_address() const {
  auto handle = dlopen(module.c_str(), RTLD_LAZY | RTLD_NOLOAD);
  if (handle == nullptr) {
    throw StringError("Failed to get handle for module {}", module);
  }

  auto addr = reinterpret_cast<uintptr_t>(dlsym(handle, symbol.c_str()));
  if (addr == 0)
    throw StringError("Failed to get address of symbol {} from module {}\n",
                      module, symbol);
  return addr;
}

namespace offsets {
  const SharedLibOffset SCR_UpdateScreen{"engine.so", 0x39eab0};
  const SharedLibOffset SND_RecordBuffer{"engine.so", 0x281410};

  const SharedLibOffset GetSoundTime{"engine.so", 0x2648c0};

  const VtableOffset CEngineSoundServices_SetSoundFrametime{
      "engine.so", "20CEngineSoundServices", 7};

  const SharedLibOffset SND_G_P{"engine.so", 0x00858910 - 0x10000};
  const SharedLibOffset SND_G_LINEAR_COUNT{"engine.so", 0x00858900 - 0x10000};
  const SharedLibOffset SND_G_VOL{"engine.so", 0x008588f0 - 0x10000};

  const SharedLibOffset FindAndHealTargets{"client.so", 0xdccea0};

  const SharedLibSymbol g_Telemetry{"libtier0.so", "g_Telemetry"};
  const SharedLibSymbol TelemetryTick{"libtier0.so", "TelemetryTick"};
  const SharedLibSymbol ThreadSetDebugName{"libtier0.so", "ThreadSetDebugName"};
  const SharedLibSymbol CVProfNode_EnterScope{"libtier0.so",
                                              "_ZN10CVProfNode10EnterScopeEv"};
  const SharedLibSymbol CVProfNode_ExitScope{"libtier0.so",
                                             "_ZN10CVProfNode9ExitScopeEv"};
  const VtableOffset CEngine_Frame{"engine.so", "7CEngine", 6};
}  // namespace offsets
