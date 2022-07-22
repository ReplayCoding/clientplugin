#include <interfaces.hpp>

#include <cdll_int.h>
#include <igameevents.h>
#include <replay/ienginereplay.h>
#include <tier1/tier1.h>
#include <tier2/tier2.h>
#include <tier3/tier3.h>
#include <toolframework/ienginetool.h>

void InterfaceManager::Load(CreateInterfaceFn factory) {
  ConnectTier1Libraries(&factory, 1);
  ConVar_Register();
  ConnectTier2Libraries(&factory, 1);
  ConnectTier3Libraries(&factory, 1);

  engineClient = static_cast<IVEngineClient013 *>(
      factory(VENGINE_CLIENT_INTERFACE_VERSION_13, nullptr));
  gameEventManager = static_cast<IGameEventManager2 *>(
      factory(INTERFACEVERSION_GAMEEVENTSMANAGER2, nullptr));
  engineClientReplay = static_cast<IEngineClientReplay *>(
      factory(ENGINE_REPLAY_CLIENT_INTERFACE_VERSION, nullptr));
	engineTool = (IEngineTool *)factory(VENGINETOOL_INTERFACE_VERSION, nullptr);
	materialSystem = (IMaterialSystem*)factory("VMaterialSystem081", nullptr);

  CreateInterfaceFn gameClientFactory;
  engineTool->GetClientFactory(gameClientFactory);

  clientDll = static_cast<IBaseClientDLL *>(
      gameClientFactory(CLIENT_DLL_INTERFACE_VERSION, nullptr));
};

void InterfaceManager::Unload() {
  engineClient = nullptr;
  gameEventManager = nullptr;
  engineClientReplay = nullptr;
  engineTool = nullptr;
  materialSystem = nullptr;
  // TODO

  DisconnectTier3Libraries();
  DisconnectTier2Libraries();
  ConVar_Unregister();
  DisconnectTier1Libraries();
};

InterfaceManager Interfaces{};
