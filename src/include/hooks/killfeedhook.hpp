#pragma once
#include <gum/interceptor.hpp>

#include <sdk.hpp>

class KillfeedListener : public Listener {
public:
  KillfeedListener();
  ~KillfeedListener();
  virtual void on_enter(GumInvocationContext *context);

  virtual void on_leave(GumInvocationContext *context);

  enum HookId : int {
    HOOK_DLOPEN,
    HOOK_FIREGAMEEVENT,
  };

private:
  // void *vfptr_bkp{};
  // void **vfptr{};
  // static void drawportals_stub(){};

  void handleGameEvent_handler(const void *thisPtr, IGameEvent *gameEvent);
};

class KillfeedHook {
public:
  KillfeedHook();
  ~KillfeedHook();

private:
  std::shared_ptr<KillfeedListener> listener;
};
