#include <cdll_int.h>
#include <igameevents.h>
#include <materialsystem/imaterialsystem.h>
#include <mathlib/mathlib.h>
#include <replay/ienginereplay.h>
#include <tier1/tier1.h>
#include <tier2/tier2.h>
#include <tier3/tier3.h>
#include <toolframework/ienginetool.h>

#include "interfaces.hpp"

namespace Interfaces {
  IVEngineClient* EngineClient;
  IGameEventManager2* GameEventManager;
  IEngineClientReplay* EngineClientReplay;
  IBaseClientDLL* ClientDll;
  IEngineTool* EngineTool;
  IMaterialSystem* MaterialSystem;

  void Load(CreateInterfaceFn factory) {
    ConnectTier1Libraries(&factory, 1);
    ConVar_Register();
    ConnectTier2Libraries(&factory, 1);
    ConnectTier3Libraries(&factory, 1);

    Interfaces::EngineClient = static_cast<IVEngineClient*>(
        factory(VENGINE_CLIENT_INTERFACE_VERSION, nullptr));
    Interfaces::GameEventManager = static_cast<IGameEventManager2*>(
        factory(INTERFACEVERSION_GAMEEVENTSMANAGER2, nullptr));
    Interfaces::EngineClientReplay = static_cast<IEngineClientReplay*>(
        factory(ENGINE_REPLAY_CLIENT_INTERFACE_VERSION, nullptr));
    Interfaces::EngineTool =
        (IEngineTool*)factory(VENGINETOOL_INTERFACE_VERSION, nullptr);
    Interfaces::MaterialSystem =
        (IMaterialSystem*)factory(MATERIAL_SYSTEM_INTERFACE_VERSION, nullptr);

    CreateInterfaceFn gameClientFactory;
    EngineTool->GetClientFactory(gameClientFactory);

    Interfaces::ClientDll = static_cast<IBaseClientDLL*>(
        gameClientFactory(CLIENT_DLL_INTERFACE_VERSION, nullptr));
  }

  void Unload() {
    Interfaces::EngineClient = nullptr;
    Interfaces::GameEventManager = nullptr;
    Interfaces::EngineClientReplay = nullptr;
    Interfaces::EngineTool = nullptr;
    Interfaces::ClientDll = nullptr;
    Interfaces::MaterialSystem = nullptr;

    DisconnectTier3Libraries();
    DisconnectTier2Libraries();
    ConVar_Unregister();
    DisconnectTier1Libraries();
  }
}  // namespace Interfaces
