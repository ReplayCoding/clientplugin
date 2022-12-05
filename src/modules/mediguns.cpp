#include <convar.h>
#include <fmt/core.h>

#include "hook/attachmenthook.hpp"
#include "modules/modules.hpp"
#include "offsets/clientclasses.hpp"
#include "offsets/offsets.hpp"

class MediGunsMod : public IModule {
 public:
  MediGunsMod();

 private:
  std::unique_ptr<AttachmentHookEnter> lazy_hook;
  float my_charge_val{};
};

MediGunsMod::MediGunsMod() {
  lazy_hook = std::make_unique<AttachmentHookEnter>(
      offsets::FindAndHealTargets, [this](InvocationContext context) {
        uintptr_t self = context.get_arg<uintptr_t>(0);
        auto charge_val_offset = g_ClientClasses->get_prop_offset(
            "CWeaponMedigun", "DT_LocalTFWeaponMedigunData::m_flChargeLevel");
        auto current_charge = *(float*)(self + charge_val_offset);
        if (current_charge < my_charge_val) {
          *(float*)(self + charge_val_offset) = my_charge_val;
          fmt::print(
              "[CLIENTPLUGIN] m_flChargeLevel is less than prev value: {} (was "
              "{})\n",
              current_charge, my_charge_val);
        }

        my_charge_val = current_charge;
      });
}

REGISTER_MODULE(MediGunsMod)
