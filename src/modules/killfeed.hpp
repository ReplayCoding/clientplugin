#pragma once
#include <memory>
#include <convar.h>

#include "modules/modules.hpp"
#include "hook/attachmenthook.hpp"

class KillfeedMod : public IModule {
 public:
  KillfeedMod();
  virtual ~KillfeedMod();

 private:
  void FireGameEvent_handler(InvocationContext context);
  std::unique_ptr<AttachmentHookEnter> fireGameEvent_attachment;
  ConVar pe_killfeed_debug;
};
