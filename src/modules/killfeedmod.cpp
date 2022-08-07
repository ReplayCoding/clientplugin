#include <gum/interceptor.hpp>
#include <interfaces.hpp>
#include <modules/killfeedmod.hpp>
#include <offsets.hpp>
#include <plugin.hpp>
#include <sdk-excluded.hpp>

#include <cdll_int.h>
#include <convar.h>
#include <igameevents.h>

static ConVar pe_killfeed_debug("pe_killfeed_debug", 0, FCVAR_NONE,
                                "Enable debugging of killfeed game events");

void KillfeedMod::on_enter(GumInvocationContext *context) {
  const auto gameEvent = static_cast<IGameEvent *>(
      gum_invocation_context_get_nth_argument(context, 1));

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

void KillfeedMod::on_leave(GumInvocationContext *context){};

KillfeedMod::KillfeedMod() : Listener() {
  const GumAddress module_base = gum_module_find_base_address("client.so");
  const uintptr_t fireGameEvent_ptr =
      module_base + offsets::FIREGAMEEVENT_OFFSET;
  g_Interceptor->attach(fireGameEvent_ptr, this, nullptr);
};
KillfeedMod::~KillfeedMod() { g_Interceptor->detach(this); };
