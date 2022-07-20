#include "modules/videorecordmod.hpp"
#include <modules.hpp>
#include <modules/killfeedmod.hpp>
#include <memory>

ModuleManager::ModuleManager() {
  modules.emplace_back(std::make_unique<KillfeedMod>());
  // modules.emplace_back(std::make_unique<VideoRecordMod>());
};
ModuleManager::~ModuleManager(){};
