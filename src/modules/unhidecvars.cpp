#include <convar.h>

#include "modules/modules.hpp"
#include "sdk/ccvar.hpp"

class UnhideCVarsMod : public IModule {
 public:
  UnhideCVarsMod();

 private:
  static void unhide_cvars();

  ConCommand unhide_cvars_cmd;
};

UnhideCVarsMod::UnhideCVarsMod()
    : unhide_cvars_cmd("fh_unhide_cvars", unhide_cvars) {}

void UnhideCVarsMod::unhide_cvars() {
  static constexpr auto MASK = ~(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);

  for (auto var = g_pCVar->GetCommands(); var; var = var->GetNext()) {
    CCvar::SetFlags(var, CCvar::GetFlags(var) & MASK);
  };
}

REGISTER_MODULE(UnhideCVarsMod)
