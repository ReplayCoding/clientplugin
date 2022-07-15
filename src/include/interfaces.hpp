#include <sdk.hpp>

class InterfaceManager {
public:
  void Load(CreateInterfaceFn factory);
  void Unload();

  auto GetEngineClient() { return engineClient; };
  auto GetGameEventManager() { return gameEventManager; };

private:
  IVEngineClient013 *engineClient{};
  IGameEventManager2 *gameEventManager{};
};

extern InterfaceManager Interfaces;
