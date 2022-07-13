#include <plugin.hpp>

// For whatever reason, system headers must be included BEFORE sdk.hpp
#include <frida-gum.h>
#include <sdk.hpp>

ServerPlugin plugin{};
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(ServerPlugin, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, plugin );
