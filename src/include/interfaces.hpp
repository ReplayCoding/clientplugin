#include <sdk.hpp>

class InterfaceManager {
public:
  void Load(CreateInterfaceFn factory);
  void Unload();

  auto GetEngineClient() {
    return engineClient;
  };
private:
  IVEngineClient013* engineClient{};
};

extern InterfaceManager Interfaces;
