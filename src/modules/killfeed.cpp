#include <cdll_int.h>
#include <convar.h>
#include <igameevents.h>
#include <functional>

#include "hook/attachmenthook.hpp"
#include "interfaces.hpp"
#include "modules/killfeed.hpp"
#include "modules/modules.hpp"
#include "offsets.hpp"
#include "sdk/tfdefs.hpp"

void KillfeedMod::FireGameEvent_handler(InvocationContext context) {
  const auto gameEvent = context.get_arg<IGameEvent*>(1);

  const auto customkill = static_cast<ETFDmgCustom>(gameEvent->GetInt(
      "customkill", static_cast<int>(ETFDmgCustom::TF_DMG_CUSTOM_NONE)));

  if (pe_killfeed_debug.GetBool()) {
    int i = 0;
    Interfaces::EngineClient->Con_NPrintf(i++, "event name: %s\n",
                                         gameEvent->GetName());
    Interfaces::EngineClient->Con_NPrintf(i++, "weapon name: %s\n",
                                         gameEvent->GetString("weapon"));
    Interfaces::EngineClient->Con_NPrintf(i++, "customkill: %d\n", customkill);
  };

  if (customkill == ETFDmgCustom::TF_DMG_CUSTOM_BACKSTAB) {
    gameEvent->SetInt("customkill",
                      static_cast<int>(ETFDmgCustom::TF_DMG_CUSTOM_NONE));
  };
}

KillfeedMod::KillfeedMod()
    : pe_killfeed_debug("pe_killfeed_debug",
                        0,
                        FCVAR_NONE,
                        "Enable debugging of killfeed game events") {
  fireGameEvent_attachment = std::make_unique<AttachmentHookEnter>(
      offsets::CHudBaseDeathNotice_FireGameEvent,
      [this](auto context) { FireGameEvent_handler(context); });
}
KillfeedMod::~KillfeedMod() {}

REGISTER_MODULE(KillfeedMod)
