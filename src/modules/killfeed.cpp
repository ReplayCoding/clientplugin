#include <cdll_int.h>
#include <convar.h>
#include <igameevents.h>
#include <functional>

#include "hook/attachmenthook.hpp"
#include "interfaces.hpp"
#include "modules/killfeed.hpp"
#include "modules/modules.hpp"
#include "offsets.hpp"
#include "plugin.hpp"
#include "sdk/tfdefs.hpp"

void KillfeedMod::FireGameEvent_handler(InvocationContext context) {
  const auto gameEvent = context.get_arg<IGameEvent*>(1);

  const int customkill = gameEvent->GetInt("customkill", TF_DMG_CUSTOM_NONE);

  if (pe_killfeed_debug.GetBool()) {
    int i = 0;
    Interfaces.engineClient->Con_NPrintf(i++, "event name: %s\n",
                                         gameEvent->GetName());
    Interfaces.engineClient->Con_NPrintf(i++, "weapon name: %s\n",
                                         gameEvent->GetString("weapon"));
    Interfaces.engineClient->Con_NPrintf(i++, "customkill: %d\n", customkill);
  };

  if (customkill == TF_DMG_CUSTOM_BACKSTAB) {
    gameEvent->SetInt("customkill", TF_DMG_CUSTOM_NONE);
  };
}

KillfeedMod::KillfeedMod()
    : pe_killfeed_debug("pe_killfeed_debug",
                        0,
                        FCVAR_NONE,
                        "Enable debugging of killfeed game events") {
  fireGameEvent_attachment = std::make_unique<AttachmentHookEnter>(
      offsets::CHudBaseDeathNotice_FireGameEvent,
      std::bind(&KillfeedMod::FireGameEvent_handler, this,
                std::placeholders::_1));
}
KillfeedMod::~KillfeedMod() {}

REGISTER_MODULE(KillfeedMod)