#include <hooks/killfeedhook.hpp>
#include <memory>
#include <plugin.hpp>

Interceptor *g_Interceptor;

bool ServerPlugin::Load(CreateInterfaceFn interfaceFactory,
                        CreateInterfaceFn gameServerFactory) {

  ConnectTier1Libraries(&interfaceFactory, 1);
  ConnectTier2Libraries(&interfaceFactory, 1);
  ConVar_Register();

  gum_init();
  g_Interceptor = new Interceptor();
  hook = std::make_unique<KillfeedHook>(interfaceFactory);
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

  delete g_Interceptor;
  gum_deinit();

  ConVar_Unregister();
  DisconnectTier2Libraries();
  DisconnectTier1Libraries();
};

ServerPlugin plugin{};
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(ServerPlugin, IServerPluginCallbacks,
                                  INTERFACEVERSION_ISERVERPLUGINCALLBACKS,
                                  plugin);
