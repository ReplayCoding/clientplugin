#include "plugin.hpp"

#include "sdk.hpp"
#include "tfdefs.hpp"

// TODO: We should search signatures
const gpointer FIREGAMEEVENT_OFFSET = reinterpret_cast<gpointer>(0x01150720);

class MyListener : public Listener {
public:
  MyListener(CreateInterfaceFn interfaceFactory) : Listener() {
    engineClient = static_cast<IVEngineClient013 *>(
        interfaceFactory(VENGINE_CLIENT_INTERFACE_VERSION_13, nullptr));
    auto vtable = (*(void ***)engineClient);
    vfptr = &vtable[75]; // CEngineClient::DrawPortals()
    gum_mprotect(vfptr, sizeof vfptr, GUM_PAGE_RW);
    vfptr_bkp = *vfptr;
    *vfptr = (void *)drawportals_stub;
    gum_mprotect(vfptr, sizeof vfptr, GUM_PAGE_READ);
  };
  ~MyListener() {
    if (vfptr != nullptr) {
      gum_mprotect(vfptr, sizeof vfptr, GUM_PAGE_RW);
      *vfptr = vfptr_bkp;
      gum_mprotect(vfptr, sizeof vfptr, GUM_PAGE_READ);
    };
  };
  virtual void on_enter(GumInvocationContext *context) {
    HookId hookid = reinterpret_cast<int>(
        gum_invocation_context_get_listener_function_data(context));
    switch (hookid) {
      case HOOK_FIREGAMEEVENT: {
        const gpointer thisPtr =
            gum_invocation_context_get_nth_argument(context, 0);
        const auto gameEvent = static_cast<IGameEvent *>(
            gum_invocation_context_get_nth_argument(context, 1));
        handleGameEvent_handler(thisPtr, gameEvent);
      }
    }
  };

  virtual void on_leave(GumInvocationContext *context) {
    HookId hookid = reinterpret_cast<int>(
        gum_invocation_context_get_listener_function_data(context));
    switch (hookid) {
      case HOOK_FIREGAMEEVENT:
        break;
    }
  };

  enum HookId: int {
    HOOK_DLOPEN,
    HOOK_FIREGAMEEVENT,
  };

private:
  // This should NOT be freed!
  IVEngineClient013 *engineClient{};

  void *vfptr_bkp{};
  void **vfptr{};

  static void drawportals_stub(){};
  void handleGameEvent_handler(const void *thisPtr, IGameEvent *gameEvent) {
    const int customkill = gameEvent->GetInt("customkill", TF_DMG_CUSTOM_NONE);
    int i = 0;
    engineClient->Con_NPrintf(i++, "FireGameEvent(this: %p, event: %p)\n",
                              thisPtr, gameEvent);
    engineClient->Con_NPrintf(i++, "\t event name: %s\n", gameEvent->GetName());
    engineClient->Con_NPrintf(i++, "\t weapon name: %s\n",
                              gameEvent->GetString("weapon"));
    engineClient->Con_NPrintf(i++, "\t customkill: %d\n", customkill);
    if (customkill == TF_DMG_CUSTOM_BACKSTAB) {
      gameEvent->SetInt("customkill", TF_DMG_CUSTOM_NONE);
    };
  };
};

bool ServerPlugin::Load(CreateInterfaceFn interfaceFactory,
                        CreateInterfaceFn gameServerFactory) {

  gum_init();
  interceptor = std::make_unique<Interceptor>();
  listener = std::make_shared<MyListener>(interfaceFactory);

  const GumAddress module_base = gum_module_find_base_address("client.so");
  const gpointer fireGameEvent_ptr = module_base + FIREGAMEEVENT_OFFSET;
  interceptor->attach(fireGameEvent_ptr, listener,
                      reinterpret_cast<void *>(MyListener::HOOK_FIREGAMEEVENT));
  return true;
};

// Called when the plugin should be shutdown
void ServerPlugin::Unload(void) {
  // gum_interceptor_end_transcation crashes when you call it in a (library)
  // destructor, and this function is used in gum_interceptor_detach, which is
  // called to cleanup loose hooks.
  // Is this really really stupid? Yes.
  // (Also REMEMBER TO *NOT* FREE INTERFACES YOU FUCKING MORON!)
  //
  interceptor.reset();
  listener.reset();
  gum_deinit();
};
