#include <frida-gum.h>
#include <cstdint>

#include "offsets.hpp"
#include "util/error.hpp"

namespace offsets {
  std::uintptr_t SharedLibOffset::get_address() const {
    auto base_address = gum_module_find_base_address(module.c_str());

    if (base_address == 0)
      throw StringError("Failed to get address of module: {}", module);
    return base_address + offset;
  }

  const SharedLibOffset SCR_UpdateScreen{"engine.so", 0x39eab0};
  const SharedLibOffset SND_RecordBuffer{"engine.so", 0x281410};

  const SharedLibOffset GetSoundTime{"engine.so", 0x2648c0};

  // This is in a vtable so we should probably fix that
  const SharedLibOffset CEngineSoundServices_SetSoundFrametime{"engine.so", 0x00387a50 - 0x10000};

  const SharedLibOffset SND_G_P{"engine.so", 0x00858910 - 0x10000};
  const SharedLibOffset SND_G_LINEAR_COUNT{"engine.so", 0x00858900 - 0x10000};
  const SharedLibOffset SND_G_VOL{"engine.so", 0x008588f0 - 0x10000};
}  // namespace offsets
