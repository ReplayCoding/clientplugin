#include <interfaces.hpp>

#include <replay/ienginereplay.h>
#include <sdk.hpp>

void InterfaceManager::Load(CreateInterfaceFn factory) {
  ConnectTier1Libraries(&factory, 1);
  ConnectTier2Libraries(&factory, 1);
  ConnectTier3Libraries(&factory, 1);

  engineClient = static_cast<IVEngineClient013 *>(
      factory(VENGINE_CLIENT_INTERFACE_VERSION_13, nullptr));
  gameEventManager = static_cast<IGameEventManager2 *>(
      factory(INTERFACEVERSION_GAMEEVENTSMANAGER2, nullptr));
  engineClientReplay = static_cast<IEngineClientReplay *>(
      factory(ENGINE_REPLAY_CLIENT_INTERFACE_VERSION), nullptr);
};

void InterfaceManager::Unload() {
  engineClient = nullptr;
  gameEventManager = nullptr;

  DisconnectTier2Libraries();
  DisconnectTier1Libraries();
  DisconnectTier3Libraries();
};

InterfaceManager Interfaces{};
