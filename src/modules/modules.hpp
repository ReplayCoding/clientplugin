#pragma once
#include <imgui.h>
#include <functional>
#include <memory>
#include <vector>

class IModule {
 public:
  virtual ~IModule() = default;

  virtual bool should_draw_overlay() { return false; };
  virtual void draw_overlay(){};

  const char* name{};
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

#define REGISTER_MODULE(M)              \
  static ModuleDesc mod_desc_##M([]() { \
    auto m = std::make_unique<M>();     \
    m->name = #M;                       \
    return m;                           \
  });
class GfxOverlayMod;
class ModuleManager {
 public:
  ModuleManager();
  ~ModuleManager();

 private:
  std::vector<std::unique_ptr<IModule>> modules;
  GfxOverlayMod* gfx_overlay;
};
