#include <gum/interceptor.hpp>
#include <modules/killfeedmod.hpp>
#include <interfaces.hpp>
#include <plugin.hpp>

#include <sdk.hpp>
#include <tfdefs.hpp>

static ConVar pe_killfeed_debug("pe_killfeed_debug", 0, FCVAR_NONE,
                                "Disable debugging of killfeed game events");

// TODO: We should search signatures
const gpointer FIREGAMEEVENT_OFFSET = reinterpret_cast<gpointer>(0x01150720);

KillfeedListener::KillfeedListener() : Listener(){};
KillfeedListener::~KillfeedListener(){};
void KillfeedListener::on_enter(GumInvocationContext *context) {
  const auto gameEvent = static_cast<IGameEvent *>(
      gum_invocation_context_get_nth_argument(context, 1));

  const int customkill = gameEvent->GetInt("customkill", TF_DMG_CUSTOM_NONE);

  if (pe_killfeed_debug.GetBool()) {
    int i = 0;
    auto engineClient = Interfaces.GetEngineClient();
    engineClient->Con_NPrintf(i++, "event name: %s\n", gameEvent->GetName());
    engineClient->Con_NPrintf(i++, "weapon name: %s\n",
                              gameEvent->GetString("weapon"));
    engineClient->Con_NPrintf(i++, "customkill: %d\n", customkill);
  };

  if (customkill == TF_DMG_CUSTOM_BACKSTAB) {
    gameEvent->SetInt("customkill", TF_DMG_CUSTOM_NONE);
  };
};

void KillfeedListener::on_leave(GumInvocationContext *context){};

KillfeedMod::KillfeedMod() {
  listener = std::make_shared<KillfeedListener>();

  const GumAddress module_base = gum_module_find_base_address("client.so");
  const gpointer fireGameEvent_ptr = module_base + FIREGAMEEVENT_OFFSET;
  g_Interceptor->attach(fireGameEvent_ptr, listener, nullptr);
};
KillfeedMod::~KillfeedMod() { g_Interceptor->detach(listener); };
