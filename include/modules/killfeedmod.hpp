#pragma once
#include <gum/interceptor.hpp>
#include <modules/modules.hpp>

class KillfeedMod : public IModule, public Listener {
 public:
  KillfeedMod();
  virtual ~KillfeedMod();
  virtual void on_enter(GumInvocationContext* context);

  virtual void on_leave(GumInvocationContext* context);
};
