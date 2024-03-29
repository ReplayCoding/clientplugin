#include <frida-gum.h>
#include <bit>
#include <cstdint>

#include "hook/gum/interceptor.hpp"
#include "hook/gum/objectwrapper.hpp"
#include "util/error.hpp"

namespace Gum {

  ProbeListener::ProbeListener()
      : GumObjectWrapper(gum_make_probe_listener(on_hit_wrapper, this, nullptr),
                         true) {}

  GumInvocationListener* ProbeListener::get_listener() {
    return get_obj();
  }

  void ProbeListener::on_hit_wrapper(GumInvocationContext* context,
                                     void* user_data) {
    auto _this = static_cast<ProbeListener*>(user_data);
    _this->on_hit(context);
  }

  CallListener::CallListener()
      : GumObjectWrapper(gum_make_call_listener(on_enter_wrapper,
                                                on_leave_wrapper,
                                                this,
                                                nullptr),
                         true) {}

  GumInvocationListener* CallListener::get_listener() {
    return get_obj();
  }

  void CallListener::on_enter_wrapper(GumInvocationContext* context,
                                      void* user_data) {
    auto _this = static_cast<CallListener*>(user_data);
    _this->on_enter(context);
  }
  void CallListener::on_leave_wrapper(GumInvocationContext* context,
                                      void* user_data) {
    auto _this = static_cast<CallListener*>(user_data);
    _this->on_leave(context);
  }

  // Interceptor
  Interceptor::Interceptor()
      : GumObjectWrapper(gum_interceptor_obtain(), false) {}

  Interceptor::~Interceptor() {
    begin_transaction();
    for (auto& listener : call_listeners) {
      detach(listener);
    }
    for (auto& listener : probe_listeners) {
      detach(listener);
    }
    end_transaction();
  }

  void Interceptor::attach(const uintptr_t address,
                           CallListener* listener,
                           void* user_data) {
    begin_transaction();

    auto retval =
        gum_interceptor_attach(get_obj(), std::bit_cast<void*>(address),
                               listener->get_listener(), user_data);
    if (retval != GUM_ATTACH_OK)
      throw StringError("Failed to attach to address {:08X} ({})", address,
                        static_cast<int>(retval));
    call_listeners.push_back(listener);

    end_transaction();
  }
  void Interceptor::detach(CallListener* listener) {
    gum_interceptor_detach(get_obj(), listener->get_listener());
  }

  void Interceptor::attach(const uintptr_t address,
                           ProbeListener* listener,
                           void* user_data) {
    begin_transaction();

    auto retval =
        gum_interceptor_attach(get_obj(), std::bit_cast<void*>(address),
                               listener->get_listener(), user_data);

    if (retval != GUM_ATTACH_OK)
      throw StringError("Failed to attach to address {:08X} ({})", address,
                        static_cast<int>(retval));
    probe_listeners.push_back(listener);

    end_transaction();
  }

  void Interceptor::detach(ProbeListener* listener) {
    gum_interceptor_detach(get_obj(), listener->get_listener());
  }

  void Interceptor::replace(const uintptr_t address,
                            const uintptr_t replacement_address,
                            void* user_data) {
    auto retval = gum_interceptor_replace(
        get_obj(), std::bit_cast<void*>(address),
        std::bit_cast<void*>(replacement_address), user_data, nullptr);

    if (retval != GUM_REPLACE_OK)
      throw StringError("Failed to replace address {:08X} ({})", address,
                        static_cast<int>(retval));
  }

  void Interceptor::revert(const uintptr_t address) {
    gum_interceptor_revert(get_obj(), std::bit_cast<void*>(address));
  }
}  // namespace Gum

std::unique_ptr<Gum::Interceptor> g_Interceptor;
