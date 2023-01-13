#include "hook/gum/interceptor.hpp"
#include "modules/modules.hpp"
#include "offsets/offsets.hpp"

class AllowLoginNoSteamMod : public IModule {
 public:
  AllowLoginNoSteamMod();
  ~AllowLoginNoSteamMod();
};

bool FinishCertCheck_replace(void* this_ /*, ... */) {
  return true;
}

AllowLoginNoSteamMod::AllowLoginNoSteamMod() {
  g_Interceptor->replace(offsets::CGameServer_FinishCertificateCheck,
                         reinterpret_cast<uintptr_t>(FinishCertCheck_replace),
                         nullptr);
}

AllowLoginNoSteamMod::~AllowLoginNoSteamMod() {
  g_Interceptor->revert(offsets::CGameServer_FinishCertificateCheck);
}

REGISTER_MODULE(AllowLoginNoSteamMod)
