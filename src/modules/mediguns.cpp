#include <convar.h>
#include <fmt/core.h>

#include "hook/attachmenthook.hpp"
#include "modules/modules.hpp"
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
        auto current_charge = *(float*)(self + 0x00000C48);
        if (current_charge < my_charge_val) {
          *(float*)(self + 0x00000C48) = my_charge_val;
          auto current_charge_2 = *(float*)(self + 0x00000C48);
          fmt::print(
              "[CLIENTPLUGIN] m_flChargeLevel is less than prev value: {} (was "
              "{}) (and we set it to {})\n",
              current_charge, my_charge_val, current_charge_2);
        }

        my_charge_val = current_charge;
      });
}

REGISTER_MODULE(MediGunsMod)
