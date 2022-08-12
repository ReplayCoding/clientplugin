#include <cdll_int.h>
#include <convar.h>
#include <igameevents.h>

#include <frida-gum.h>
#include <functional>
#include <hook/attachmenthook.hpp>
#include <interfaces.hpp>
#include <modules/killfeedmod.hpp>
#include <modules/modules.hpp>
#include <offsets.hpp>
#include <plugin.hpp>
#include <sdk-excluded.hpp>

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
};

KillfeedMod::KillfeedMod()
    : pe_killfeed_debug("pe_killfeed_debug",
                        0,
                        FCVAR_NONE,
                        "Enable debugging of killfeed game events") {
  const GumAddress module_base = gum_module_find_base_address("client.so");
  const uintptr_t fireGameEvent_ptr =
      module_base + offsets::FIREGAMEEVENT_OFFSET;
  fireGameEvent_attachment = std::make_unique<AttachmentHookEnter>(
      fireGameEvent_ptr, std::bind(&KillfeedMod::FireGameEvent_handler, this,
                                   std::placeholders::_1));
};
KillfeedMod::~KillfeedMod(){};

REGISTER_MODULE(KillfeedMod);
