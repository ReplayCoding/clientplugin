#include "modules/videorecordmod.hpp"
#include <memory>
#include <modules/modules.hpp>
#include <modules/killfeedmod.hpp>

ModuleManager::ModuleManager() {
  modules.emplace_back(std::make_unique<KillfeedMod>());
  modules.emplace_back(std::make_unique<VideoRecordMod>());
};
ModuleManager::~ModuleManager(){};
