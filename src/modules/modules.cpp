#include <memory>
#include <modules/gfxoverlaymod.hpp>
#include <modules/killfeedmod.hpp>
#include <modules/modules.hpp>
#include <modules/videorecordmod.hpp>

ModuleManager::ModuleManager() {
  gfxOverlayModule = std::make_unique<GfxOverlayMod>();
  killfeedModule = std::make_unique<KillfeedMod>();
  videoRecordModule = std::make_unique<VideoRecordMod>();
};
ModuleManager::~ModuleManager(){};
