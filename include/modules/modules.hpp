#pragma once
#include <memory>
#include <vector>

class IModule {
 public:
  IModule(){};
  virtual ~IModule(){};
};

class ModuleManager {
 public:
  ModuleManager();
  ~ModuleManager();

 private:
  std::unique_ptr<IModule> killfeedModule;
  std::unique_ptr<IModule> videoRecordModule;
  std::unique_ptr<IModule> gfxOverlayModule;
};
