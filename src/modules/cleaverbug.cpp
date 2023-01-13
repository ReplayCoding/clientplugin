#include <convar.h>
#include <engine/ivdebugoverlay.h>
#include <fmt/core.h>
#include <mathlib/mathlib.h>
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
  std::unique_ptr<AttachmentHookEnter> cleaver_create_projectile_hook;
};

auto box_size = Vector{8, 8, 8};

JarBugMod::JarBugMod() {
  cleaver_create_projectile_hook = std::make_unique<AttachmentHookEnter>(
      offsets::CTFCleaver_CreateJarProjectile, [](InvocationContext context) {
        auto position = context.get_arg<Vector*>(1);
        auto velocity = context.get_arg<Vector*>(3);
        Interfaces::DebugOverlay->AddBoxOverlay(
            *position, -box_size, box_size, QAngle{0, 0, 0}, 255, 0, 0, 64, 30);
        Interfaces::DebugOverlay->AddBoxOverlay(
            *position + *velocity, -box_size, box_size, QAngle{0, 0, 0}, 0, 0,
            255, 64, 30);
      });
}

REGISTER_MODULE(JarBugMod)
