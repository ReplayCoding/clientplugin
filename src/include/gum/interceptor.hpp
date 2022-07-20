#pragma once
#include <frida-gum.h>
#include <memory>
#include <set>

#include "objectwrapper.hpp"

class Listener : public GumObjectWrapper<GumInvocationListener> {
public:
  Listener();
  virtual ~Listener();
  GumInvocationListener *get_listener();

  virtual void on_enter(GumInvocationContext *context) = 0;
  virtual void on_leave(GumInvocationContext *context) = 0;

private:
  static void on_enter_wrapper(GumInvocationContext *context, void *user_data);
  static void on_leave_wrapper(GumInvocationContext *context, void *user_data);
};

class Interceptor : GumObjectWrapper<GumInterceptor> {
public:
  Interceptor();
  ~Interceptor();
  GumAttachReturn attach(void *address, Listener* listener,
                         void *user_data);
  void detach(Listener* listener, bool erase = true);

  GumReplaceReturn replace(void *address, void *replacement_address,
                           void *user_data);
  void revert(void *address);

  inline void begin_transaction() {
    gum_interceptor_begin_transaction(get_obj());
  };
  inline void end_transaction() { gum_interceptor_end_transaction(get_obj()); };

private:
  std::set<Listener *> listeners{};
};
