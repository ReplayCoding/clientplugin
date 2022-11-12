#include <convar.h>

#include "hook/attachmenthook.hpp"
#include "modules/modules.hpp"

class MediGunsMod : public IModule {
 public:
  MediGunsMod();

 private:
  std::unique_ptr<AttachmentHookEnter> lazy_hook;
  float my_charge_val{};
};
