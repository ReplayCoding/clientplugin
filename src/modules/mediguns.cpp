#include <convar.h>
#include <fmt/core.h>
#include <bit>

#include "hook/attachmenthook.hpp"
#include "modules/modules.hpp"
#include "offsets/clientclasses.hpp"
#include "offsets/offsets.hpp"

class MediGunsMod : public IModule {
 public:
  MediGunsMod();

 private:
  std::unique_ptr<AttachmentHookBoth> lazy_hook;
  float charge_val = 0;
};

MediGunsMod::MediGunsMod() {
  auto charge_val_offset = g_ClientClasses->get_prop_offset(
      "CWeaponMedigun", "DT_LocalTFWeaponMedigunData::m_flChargeLevel");
  lazy_hook = std::make_unique<AttachmentHookBoth>(
      offsets::FindAndHealTargets,
      [this, charge_val_offset](InvocationContext context) {
        uintptr_t self = context.get_arg<uintptr_t>(0);
        charge_val = *std::bit_cast<float*>(self + charge_val_offset);
      },
      [this, charge_val_offset](InvocationContext context) {
        uintptr_t self = context.get_arg<uintptr_t>(0);
        auto current_charge = *std::bit_cast<float*>(self + charge_val_offset);
        fmt::print("OLD: {} NEW: {}\n", charge_val, current_charge);
      });
}

REGISTER_MODULE(MediGunsMod)
