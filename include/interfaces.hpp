#pragma once

#include <interface.h>

class IVEngineClient;
class IGameEventManager2;
class IEngineClientReplay;
class IBaseClientDLL;
class IEngineTool;
class IMaterialSystem;

class InterfaceManager {
public:
  void Load(CreateInterfaceFn factory);
  void Unload();

  IVEngineClient *engineClient{};
  IGameEventManager2 *gameEventManager{};
  IEngineClientReplay *engineClientReplay{};
  IBaseClientDLL *clientDll{};
  IEngineTool *engineTool{};
  IMaterialSystem *materialSystem{};
};

extern InterfaceManager Interfaces;
