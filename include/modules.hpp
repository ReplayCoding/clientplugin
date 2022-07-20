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
  std::vector<std::unique_ptr<IModule>> modules;
};
