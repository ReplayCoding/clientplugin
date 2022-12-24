#pragma once

#include <interface.h>

class IVEngineClient;
class IGameEventManager2;
class IEngineClientReplay;
class IBaseClientDLL;
class IEngineTool;
class IMaterialSystem;
class IPlayerInfoManager;

namespace Interfaces {
  void Load(CreateInterfaceFn factory);
  void Unload();

  extern IVEngineClient* EngineClient;
  extern IGameEventManager2* GameEventManager;
  extern IEngineClientReplay* EngineClientReplay;
  extern IBaseClientDLL* ClientDll;
  extern IEngineTool* EngineTool;
  extern IMaterialSystem* MaterialSystem;
  extern IPlayerInfoManager* PlayerInfoManager;
}  // namespace Interfaces
