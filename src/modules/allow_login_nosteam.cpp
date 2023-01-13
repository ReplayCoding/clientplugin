#include <memory>

#include "hook/hook.hpp"
#include "modules/modules.hpp"
#include "offsets/offsets.hpp"

class AllowLoginNoSteamMod : public IModule {
 public:
  AllowLoginNoSteamMod();

  std::unique_ptr<ReplacementHook> finish_cert_check_hook;
};

bool FinishCertCheck_replace(void* this_ /*, ... */) {
  return true;
}

AllowLoginNoSteamMod::AllowLoginNoSteamMod() {
  finish_cert_check_hook = std::make_unique<ReplacementHook>(
      offsets::CGameServer_FinishCertificateCheck,
      reinterpret_cast<uintptr_t>(FinishCertCheck_replace));
}

REGISTER_MODULE(AllowLoginNoSteamMod)
