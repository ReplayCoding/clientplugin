#include <frida-gum.h>

#include <gum/interceptor.hpp>
#include <gum/objectwrapper.hpp>

namespace Gum {

  ProbeListener::ProbeListener()
      : GumObjectWrapper(gum_make_probe_listener(on_hit_wrapper, this, nullptr),
                         true){};
  ProbeListener::~ProbeListener(){};
  GumInvocationListener* ProbeListener::get_listener() {
    return get_obj();
  };

  void ProbeListener::on_hit_wrapper(GumInvocationContext* context,
                                     void* user_data) {
    auto _this = static_cast<ProbeListener*>(user_data);
    _this->on_hit(context);
  };

  CallListener::CallListener()
      : GumObjectWrapper(gum_make_call_listener(on_enter_wrapper,
                                                on_leave_wrapper,
                                                this,
                                                nullptr),
                         true){};
  CallListener::~CallListener(){};
  GumInvocationListener* CallListener::get_listener() {
    return get_obj();
  };

  void CallListener::on_enter_wrapper(GumInvocationContext* context,
                                      void* user_data) {
    auto _this = static_cast<CallListener*>(user_data);
    _this->on_enter(context);
  };
  void CallListener::on_leave_wrapper(GumInvocationContext* context,
                                      void* user_data) {
    auto _this = static_cast<CallListener*>(user_data);
    _this->on_leave(context);
  };

  // Interceptor
  Interceptor::Interceptor()
      : GumObjectWrapper(gum_interceptor_obtain(), false){};
  Interceptor::~Interceptor() {
    for (auto& listener : call_listeners) {
      detach(listener, false);
    }
    for (auto& listener : probe_listeners) {
      detach(listener, false);
    }
  };

  GumAttachReturn Interceptor::attach(void* address,
                                      CallListener* listener,
                                      void* user_data) {
    auto retval = gum_interceptor_attach(get_obj(), address,
                                         listener->get_listener(), user_data);
    if (retval == GUM_ATTACH_OK) {
      call_listeners.insert(listener);
    };
    return retval;
  };
  void Interceptor::detach(CallListener* listener, bool erase) {
    gum_interceptor_detach(get_obj(), listener->get_listener());
    if (erase) {
      call_listeners.erase(listener);
    };
  };

  GumAttachReturn Interceptor::attach(void* address,
                                      ProbeListener* listener,
                                      void* user_data) {
    auto retval = gum_interceptor_attach(get_obj(), address,
                                         listener->get_listener(), user_data);
    if (retval == GUM_ATTACH_OK) {
      probe_listeners.insert(listener);
    };
    return retval;
  };
  void Interceptor::detach(ProbeListener* listener, bool erase) {
    gum_interceptor_detach(get_obj(), listener->get_listener());
    if (erase) {
      probe_listeners.erase(listener);
    };
  };

  GumReplaceReturn Interceptor::replace(void* address,
                                        void* replacement_address,
                                        void* user_data) {
    return gum_interceptor_replace(get_obj(), address, replacement_address,
                                   user_data);
  };
  void Interceptor::revert(void* address) {
    gum_interceptor_revert(get_obj(), address);
  };
}  // namespace Gum
