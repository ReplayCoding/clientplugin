#include <hooks/killfeedhook.hpp>
#include <interfaces.hpp>
#include <memory>
#include <plugin.hpp>

Interceptor *g_Interceptor;

bool ServerPlugin::Load(CreateInterfaceFn interfaceFactory,
                        CreateInterfaceFn gameServerFactory) {

  gum_init();

  Interfaces.Load(interfaceFactory);
  ConVar_Register();

  g_Interceptor = new Interceptor();

  hook = std::make_unique<KillfeedHook>();
  return true;
};

// Called when the plugin should be shutdown
void ServerPlugin::Unload(void) {
  // gum_interceptor_end_transcation crashes when you call it in a (library)
  // destructor, and this function is used in gum_interceptor_detach, which is
  // called to cleanup loose hooks.
  // Is this really really stupid? Yes.
  // (TLDR: Reset all hooks in here, or else they will be freed in a library
  // destructor, which will crash)
  //
  // Also REMEMBER TO *NOT* FREE INTERFACES YOU FUCKING MORON!

  hook.reset();

  Interfaces.Unload();
  ConVar_Unregister();

  delete g_Interceptor;
  gum_deinit();
};

ServerPlugin plugin{};
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(ServerPlugin, IServerPluginCallbacks,
                                  INTERFACEVERSION_ISERVERPLUGINCALLBACKS,
                                  plugin);
