#include <convar.h>
#include <interface.h>
#include <unistd.h>
#include <memory>

#include "clientclasses.hpp"
#include "hook/gum/interceptor.hpp"
#include "interfaces.hpp"
#include "modules/modules.hpp"
#include "plugin.hpp"
#include "rtti.hpp"

// Defined in hook/gum/interceptor.hpp
std::unique_ptr<Gum::Interceptor> g_Interceptor;

bool ServerPlugin::Load(CreateInterfaceFn interfaceFactory,
                        CreateInterfaceFn gameServerFactory) {
  // FIXME: This is terrible
  printf("Waiting to attach BIG CHUNGUS!\n");
  sleep(3);

  gum_init_embedded();
  Interfaces::Load(interfaceFactory);

  g_Interceptor = std::make_unique<Gum::Interceptor>();
  module_manager = std::make_unique<ModuleManager>();
  client_class_manager = std::make_unique<ClientClassManager>();
  LoadRtti();

  return true;
}

// Called when the plugin should be shutdown
void ServerPlugin::Unload(void) {
  // gum_interceptor_end_transaction crashes when you call it in a (library)
  // destructor, and this function is used in gum_interceptor_detach, which is
  // called to cleanup loose hooks.
  // Is this really really stupid? Yes.
  // (TLDR: Reset all hooks in here, or else they will be freed in a library
  // destructor, which will crash)

  module_manager.reset();
  client_class_manager.reset();
  g_Interceptor.reset();

  Interfaces::Unload();
  gum_deinit_embedded();
}

ServerPlugin plugin{};
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(ServerPlugin,
                                  IServerPluginCallbacks,
                                  INTERFACEVERSION_ISERVERPLUGINCALLBACKS,
                                  plugin)
