#pragma once

#include <interface.h>

class IVEngineClient013;
class IGameEventManager2;
class IEngineClientReplay;

class InterfaceManager {
public:
  void Load(CreateInterfaceFn factory);
  void Unload();

  auto GetEngineClient() { return engineClient; };
  auto GetGameEventManager() { return gameEventManager; };
  auto GetEngineClientReplay() { return engineClientReplay; };

private:
  IVEngineClient013 *engineClient{};
  IGameEventManager2 *gameEventManager{};
  IEngineClientReplay *engineClientReplay{};
};

extern InterfaceManager Interfaces;
