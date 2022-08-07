#include <interfaces.hpp>
#include <memory>
#include <modules/killfeedmod.hpp>
#include <modules/modules.hpp>
#include <plugin.hpp>

#include <convar.h>
#include <interface.h>

std::unique_ptr<Interceptor> g_Interceptor;

bool ServerPlugin::Load(CreateInterfaceFn interfaceFactory,
                        CreateInterfaceFn gameServerFactory) {

  gum_init_embedded();
  Interfaces.Load(interfaceFactory);

  g_Interceptor = std::make_unique<Interceptor>();
  moduleManager = std::make_unique<ModuleManager>();

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

  moduleManager.reset();
  Interfaces.Unload();

  g_Interceptor.reset();
  gum_deinit_embedded();
};

ServerPlugin plugin{};
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(ServerPlugin, IServerPluginCallbacks,
                                  INTERFACEVERSION_ISERVERPLUGINCALLBACKS,
                                  plugin);
