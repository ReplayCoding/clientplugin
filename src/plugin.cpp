#include <convar.h>
#include <interface.h>
#include <unistd.h>
#include <memory>

#include "clientclasses.hpp"
#include "hook/gum/interceptor.hpp"
#include "interfaces.hpp"
#include "modules/modules.hpp"
#include "offsets/rtti.hpp"
#include "plugin.hpp"

#include <fmt/core.h>
#include "offsets/offsets.hpp"

bool ServerPlugin::Load(CreateInterfaceFn interfaceFactory,
                        CreateInterfaceFn gameServerFactory) {
  // FIXME: This is terrible
  // printf("Waiting to attach BIG CHUNGUS!\n");
  // sleep(3);

  gum_init_embedded();
  g_Interceptor = std::make_unique<Gum::Interceptor>();

  Interfaces::Load(interfaceFactory);

  g_RTTI = std::make_unique<RttiManager>();

  fmt::print("POOTIS IS AT! {:08X}\n",
             static_cast<std::uintptr_t>(
                 offsets::CEngineSoundServices_SetSoundFrametime));

  client_class_manager = std::make_unique<ClientClassManager>();
  module_manager = std::make_unique<ModuleManager>();

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

  g_RTTI.reset();

  Interfaces::Unload();

  g_Interceptor.reset();
  gum_deinit_embedded();
}

ServerPlugin plugin{};
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(ServerPlugin,
                                  IServerPluginCallbacks,
                                  INTERFACEVERSION_ISERVERPLUGINCALLBACKS,
                                  plugin)
