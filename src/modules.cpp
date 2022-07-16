#include <modules.hpp>
#include <modules/killfeedmod.hpp>
#include <memory>

ModuleManager::ModuleManager() {
  modules.emplace_back(std::make_unique<KillfeedMod>());
};
ModuleManager::~ModuleManager(){};
