#include <convar.h>

#include "modules/modules.hpp"
#include "modules/unhidecvars.hpp"
#include "sdk/ccvar.hpp"

UnhideCVarsMod::UnhideCVarsMod()
    : pe_unhide_cvars("pe_unhide_cvars", unhide_cvars) {}

void UnhideCVarsMod::unhide_cvars() {
  static constexpr auto MASK = ~(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);

  for (auto var = g_pCVar->GetCommands(); var; var = var->GetNext()) {
    CCvar::SetFlags(var, CCvar::GetFlags(var) & MASK);
  };
}

REGISTER_MODULE(UnhideCVarsMod)
