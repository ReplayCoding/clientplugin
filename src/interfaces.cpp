#include <cdll_int.h>
#include <interfaces.hpp>

#include <sdk.hpp>

void InterfaceManager::Load(CreateInterfaceFn factory) {
  ConnectTier1Libraries(&factory, 1);
  ConnectTier2Libraries(&factory, 1);

  engineClient = static_cast<IVEngineClient013 *>(
      factory(VENGINE_CLIENT_INTERFACE_VERSION_13, nullptr));
};

void InterfaceManager::Unload() {
  engineClient = nullptr;

  DisconnectTier2Libraries();
  DisconnectTier1Libraries();
};

InterfaceManager Interfaces{};
