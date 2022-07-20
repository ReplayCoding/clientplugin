#pragma once
#include <gum/interceptor.hpp>
#include <modules.hpp>

class KillfeedListener : public Listener {
public:
  KillfeedListener();
  ~KillfeedListener();
  virtual void on_enter(GumInvocationContext *context);

  virtual void on_leave(GumInvocationContext *context);
};

class KillfeedMod : public IModule {
public:
  KillfeedMod();
  virtual ~KillfeedMod();

private:
  KillfeedListener listener;
};
