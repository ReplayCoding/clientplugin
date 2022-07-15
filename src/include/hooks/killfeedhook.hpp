#pragma once
#include <gum/interceptor.hpp>

#include <sdk.hpp>

class KillfeedListener : public Listener {
public:
  KillfeedListener(CreateInterfaceFn interfaceFactory);
  ~KillfeedListener();
  virtual void on_enter(GumInvocationContext *context);

  virtual void on_leave(GumInvocationContext *context);

  enum HookId : int {
    HOOK_DLOPEN,
    HOOK_FIREGAMEEVENT,
  };

private:
  // This should NOT be freed!
  IVEngineClient013 *engineClient{};

  // void *vfptr_bkp{};
  // void **vfptr{};
  // static void drawportals_stub(){};

  void handleGameEvent_handler(const void *thisPtr, IGameEvent *gameEvent);
};

class KillfeedHook {
public:
  KillfeedHook(CreateInterfaceFn interfaceFactory) {
    Enable(interfaceFactory);
  };
  ~KillfeedHook() { Disable(); };

  void Enable(CreateInterfaceFn interfaceFactory);
  void Disable();

private:
  std::shared_ptr<KillfeedListener> listener;
};
