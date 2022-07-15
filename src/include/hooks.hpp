#pragma once
#include <vector>
#include <memory>

class IHook {
public:
  IHook(){};
  virtual ~IHook(){};
};

class HookManager {
public:
  HookManager();
  ~HookManager();

private:
  std::vector<std::unique_ptr<IHook>> hooks;
};
