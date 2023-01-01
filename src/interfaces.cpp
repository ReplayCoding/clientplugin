#include <cdll_int.h>
#include <eiface.h>
#include <igameevents.h>
#include <materialsystem/imaterialsystem.h>
#include <mathlib/mathlib.h>
#include <replay/ienginereplay.h>
#include <tier1/tier1.h>
#include <tier2/tier2.h>
#include <tier3/tier3.h>
#include <toolframework/ienginetool.h>

// Very well written code.
// I can't fucking wait for module support in CMake...
#include <edict.h>
#include <game/server/iplayerinfo.h>

#include "interfaces.hpp"

namespace Interfaces {
  IVEngineClient* EngineClient;
  IVEngineServer* EngineServer;
  IGameEventManager2* GameEventManager;
  IEngineClientReplay* EngineClientReplay;
  IBaseClientDLL* ClientDll;
  IEngineTool* EngineTool;
  IMaterialSystem* MaterialSystem;
  IPlayerInfoManager* PlayerInfoManager;

  void Load(CreateInterfaceFn factory) {
    ConnectTier1Libraries(&factory, 1);
    ConVar_Register();
    ConnectTier2Libraries(&factory, 1);
    ConnectTier3Libraries(&factory, 1);

    Interfaces::EngineClient = static_cast<IVEngineClient*>(
        factory(VENGINE_CLIENT_INTERFACE_VERSION, nullptr));
    Interfaces::EngineServer = static_cast<IVEngineServer*>(
        factory(INTERFACEVERSION_VENGINESERVER, nullptr));
    Interfaces::GameEventManager = static_cast<IGameEventManager2*>(
        factory(INTERFACEVERSION_GAMEEVENTSMANAGER2, nullptr));
    Interfaces::EngineClientReplay = static_cast<IEngineClientReplay*>(
        factory(ENGINE_REPLAY_CLIENT_INTERFACE_VERSION, nullptr));
    Interfaces::EngineTool = static_cast<IEngineTool*>(
        factory(VENGINETOOL_INTERFACE_VERSION, nullptr));
    Interfaces::MaterialSystem = static_cast<IMaterialSystem*>(
        factory(MATERIAL_SYSTEM_INTERFACE_VERSION, nullptr));

    CreateInterfaceFn game_client_factory;
    CreateInterfaceFn game_server_factory;
    EngineTool->GetClientFactory(game_client_factory);
    EngineTool->GetServerFactory(game_server_factory);

    Interfaces::ClientDll = static_cast<IBaseClientDLL*>(
        game_client_factory(CLIENT_DLL_INTERFACE_VERSION, nullptr));
    Interfaces::PlayerInfoManager = static_cast<IPlayerInfoManager*>(
        game_server_factory(INTERFACEVERSION_PLAYERINFOMANAGER, nullptr));
  }

  void Unload() {
    DisconnectTier3Libraries();
    DisconnectTier2Libraries();
    ConVar_Unregister();
    DisconnectTier1Libraries();
  }
}  // namespace Interfaces
