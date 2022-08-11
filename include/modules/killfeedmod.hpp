#pragma once
#include <hook/attachmenthook.hpp>
#include <hook/gum/interceptor.hpp>
#include <memory>
#include <modules/modules.hpp>

class KillfeedMod : public IModule {
 public:
  KillfeedMod();
  virtual ~KillfeedMod();

 private:
  void FireGameEvent_handler(GumInvocationContext* context);
  std::unique_ptr<AttachmentHookEnter> fireGameEvent_attachment;
};
