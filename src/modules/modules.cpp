#include <memory>

#include "modules/modules.hpp"

ModuleDesc* g_ModuleList = nullptr;

ModuleManager::ModuleManager() {
  for (auto module_desc = g_ModuleList; module_desc;
       module_desc = module_desc->next) {
    auto module = module_desc->factory();
    modules.emplace_back(std::move(module));
  }
};
