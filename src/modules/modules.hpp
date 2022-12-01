#pragma once
#include <functional>
#include <memory>
#include <vector>

class IModule {
 public:
  virtual ~IModule() = default;
};

class ModuleDesc;
extern ModuleDesc* g_ModuleList;
class ModuleDesc {
 public:
  ModuleDesc(std::function<std::unique_ptr<IModule>()> factory)
      : factory(factory) {
    this->next = g_ModuleList;
    g_ModuleList = this;
  };

  ModuleDesc* next;
  std::function<std::unique_ptr<IModule>()> factory;
};

#define REGISTER_MODULE(M) \
  static ModuleDesc mod_desc_##M([]() { return std::make_unique<M>(); });

class ModuleManager {
 public:
  ModuleManager();

 private:
  std::vector<std::unique_ptr<IModule>> modules;
};
