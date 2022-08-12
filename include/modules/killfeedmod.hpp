#pragma once
#include <hook/attachmenthook.hpp>
#include <memory>
#include <modules/modules.hpp>

class KillfeedMod : public IModule {
 public:
  KillfeedMod();
  virtual ~KillfeedMod();

 private:
  void FireGameEvent_handler(InvocationContext context);
  std::unique_ptr<AttachmentHookEnter> fireGameEvent_attachment;
};
