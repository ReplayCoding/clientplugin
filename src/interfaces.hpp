#pragma once

#include <interface.h>

class IVEngineClient;
class IVEngineServer;
class IGameEventManager2;
class IEngineClientReplay;
class IBaseClientDLL;
class IEngineTool;
class IMaterialSystem;
class IPlayerInfoManager;
class IVDebugOverlay;

namespace Interfaces {
  void Load(CreateInterfaceFn factory);
  void Unload();

  extern IVEngineClient* EngineClient;
  extern IVEngineServer* EngineServer;
  extern IGameEventManager2* GameEventManager;
  extern IEngineClientReplay* EngineClientReplay;
  extern IBaseClientDLL* ClientDll;
  extern IEngineTool* EngineTool;
  extern IMaterialSystem* MaterialSystem;
  extern IPlayerInfoManager* PlayerInfoManager;
  extern IVDebugOverlay* DebugOverlay;
}  // namespace Interfaces
