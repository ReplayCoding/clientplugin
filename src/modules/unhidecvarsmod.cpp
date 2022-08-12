#include <convar.h>
#include <ccvar.hpp>
#include <modules/unhidecvarsmod.hpp>

UnhideCVarsMod::UnhideCVarsMod()
    : pe_unhide_cvars("pe_unhide_cvars", unhideCVars){};

void UnhideCVarsMod::unhideCVars() {
  static constexpr auto MASK = ~(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);

  for (auto var = g_pCVar->GetCommands(); var; var = var->GetNext()) {
    CCvar::SetFlags(var, CCvar::GetFlags(var) & MASK);
  };
};
