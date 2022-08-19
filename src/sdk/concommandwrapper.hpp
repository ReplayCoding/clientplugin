#pragma once
#include <convar.h>
#include <functional>

class ConCommandCallbacks : public ICommandCallback {
 public:
  ConCommandCallbacks(std::function<void(const CCommand&)> callback)
      : callback(callback) {}

 private:
  void CommandCallback(const CCommand& command) override { callback(command); }

  std::function<void(const CCommand&)> callback;
};
