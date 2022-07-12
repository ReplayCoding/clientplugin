#include <plugin.hpp>

// For whatever reason, system headers must be included BEFORE sdk.hpp
#include <sdk.hpp>
#include <frida-gum.h>

ServerPlugin plugin{};

// We can't use the EXPOSE_*_INTERFACE_* macros because they require some link
// libraries that are only available for windows in the SDK
extern "C" __attribute__((__visibility__("default"))) void *
CreateInterface(const char *pName, int *pReturnCode) {
  g_print("Initing %s\n", pName);
  if (strncmp(pName, INTERFACEVERSION_ISERVERPLUGINCALLBACKS,
              strlen(INTERFACEVERSION_ISERVERPLUGINCALLBACKS)) == 0) {
    return &plugin;
  }
  return nullptr;
};
