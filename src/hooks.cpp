#include <hooks.hpp>
#include <hooks/killfeedhook.hpp>
#include <memory>

HookManager::HookManager() {
  hooks.emplace_back(std::make_unique<KillfeedHook>());
};
HookManager::~HookManager(){};
