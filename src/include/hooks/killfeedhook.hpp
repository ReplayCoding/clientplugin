#pragma once
#include <gum/interceptor.hpp>

#include <sdk.hpp>

class KillfeedListener : public Listener {
public:
  KillfeedListener();
  ~KillfeedListener();
  virtual void on_enter(GumInvocationContext *context);

  virtual void on_leave(GumInvocationContext *context);
};

class KillfeedHook {
public:
  KillfeedHook();
  ~KillfeedHook();

private:
  std::shared_ptr<KillfeedListener> listener;
};
