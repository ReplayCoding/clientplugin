#include <convar.h>
#include <engine/ivdebugoverlay.h>
#include <fmt/core.h>
#include <mathlib/vector.h>
#include <bit>
#include <memory>

#include "hook/attachmenthook.hpp"
#include "interfaces.hpp"
#include "modules/modules.hpp"
#include "offsets/offsets.hpp"

class JarBugMod : public IModule {
 public:
  JarBugMod();

 private:
  std::unique_ptr<AttachmentHookEnter> create_projectile_hook;
};

JarBugMod::JarBugMod() {
  create_projectile_hook = std::make_unique<AttachmentHookEnter>(
      offsets::CTFCleaver_CreateJarProjectile, [](InvocationContext context) {
        auto position = context.get_arg<Vector*>(1);
        fmt::print("POS: {} {} {} \n", position->x, position->y, position->z);
        Interfaces::DebugOverlay->AddBoxOverlay(
            *position, -Vector{1, 1, 1}, Vector{1, 1, 1}, QAngle{0, 0, 0}, 255,
            0, 0, 64, 10);
      });
}

REGISTER_MODULE(JarBugMod)
