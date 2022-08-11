#include <frida-gum.h>

#include <gum/interceptor.hpp>
#include <gum/objectwrapper.hpp>

namespace Gum {
  // Listener
  Listener::Listener()
      : GumObjectWrapper(gum_make_call_listener(on_enter_wrapper,
                                                on_leave_wrapper,
                                                this,
                                                nullptr),
                         true){};
  Listener::~Listener(){};
  GumInvocationListener* Listener::get_listener() {
    return get_obj();
  };

  void Listener::on_enter_wrapper(GumInvocationContext* context,
                                  void* user_data) {
    auto _this = static_cast<Listener*>(user_data);
    _this->on_enter(context);
  };
  void Listener::on_leave_wrapper(GumInvocationContext* context,
                                  void* user_data) {
    auto _this = static_cast<Listener*>(user_data);
    _this->on_leave(context);
  };

  // Interceptor
  Interceptor::Interceptor()
      : GumObjectWrapper(gum_interceptor_obtain(), false){};
  Interceptor::~Interceptor() {
    for (auto& listener : listeners) {
      detach(listener, false);
    }
  };

  GumAttachReturn Interceptor::attach(void* address,
                                      Listener* listener,
                                      void* user_data) {
    auto retval = gum_interceptor_attach(get_obj(), address,
                                         listener->get_listener(), user_data);
    if (retval == GUM_ATTACH_OK) {
      listeners.insert(listener);
    };
    return retval;
  };
  void Interceptor::detach(Listener* listener, bool erase) {
    gum_interceptor_detach(get_obj(), listener->get_listener());
    if (erase) {
      listeners.erase(listener);
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
