#include <memory>

#include "modules/gfxoverlay.hpp"
#include "modules/modules.hpp"

ModuleDesc* g_ModuleList = nullptr;

ModuleManager::ModuleManager() {
  for (auto module_desc = g_ModuleList; module_desc != nullptr;
       module_desc = module_desc->next) {
    modules.push_back(module_desc->factory());
  }

  gfx_overlay = new GfxOverlayMod(&modules);
}

ModuleManager::~ModuleManager() {
  delete gfx_overlay;
  modules.clear();
}
