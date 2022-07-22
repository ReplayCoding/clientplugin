#pragma once

#include <interface.h>

class IVEngineClient013;
class IGameEventManager2;
class IEngineClientReplay;
class IBaseClientDLL;
class IEngineTool;
class IMaterialSystem;

class InterfaceManager {
public:
  void Load(CreateInterfaceFn factory);
  void Unload();

  auto GetEngineClient() { return engineClient; };
  auto GetGameEventManager() { return gameEventManager; };
  auto GetEngineClientReplay() { return engineClientReplay; };
  auto GetClientDll() { return clientDll; };
  auto GetEngineTool() { return engineTool; };
  auto GetMaterialSystem() { return materialSystem; };

private:
  IVEngineClient013 *engineClient{};
  IGameEventManager2 *gameEventManager{};
  IEngineClientReplay *engineClientReplay{};
  IBaseClientDLL *clientDll{};
  IEngineTool *engineTool{};
  IMaterialSystem *materialSystem{};
};

extern InterfaceManager Interfaces;
