#pragma once
#include <frida-gum.h>
#include <cstdint>
#include <memory>
#include <set>

#include "objectwrapper.hpp"

namespace Gum {
  class ProbeListener : public GumObjectWrapper<GumInvocationListener> {
   public:
    ProbeListener();
    GumInvocationListener* get_listener();

    virtual void on_hit(GumInvocationContext* context) = 0;

   private:
    static void on_hit_wrapper(GumInvocationContext* context, void* user_data);
  };

  class CallListener : public GumObjectWrapper<GumInvocationListener> {
   public:
    CallListener();
    GumInvocationListener* get_listener();

    virtual void on_enter(GumInvocationContext* context) = 0;
    virtual void on_leave(GumInvocationContext* context) = 0;

   private:
    static void on_enter_wrapper(GumInvocationContext* context,
                                 void* user_data);
    static void on_leave_wrapper(GumInvocationContext* context,
                                 void* user_data);
  };

  class Interceptor : GumObjectWrapper<GumInterceptor> {
   public:
    Interceptor();
    ~Interceptor();
    GumAttachReturn attach(const std::uintptr_t address,
                           CallListener* listener,
                           void* user_data);
    GumAttachReturn attach(const std::uintptr_t address,
                           ProbeListener* listener,
                           void* user_data);

    void detach(CallListener* listener);
    void detach(ProbeListener* listener);

    GumReplaceReturn replace(const std::uintptr_t address,
                             const std::uintptr_t replacement_address,
                             void* user_data);
    void revert(const std::uintptr_t address);

    inline void begin_transaction() {
      gum_interceptor_begin_transaction(get_obj());
    };
    inline void end_transaction() {
      gum_interceptor_end_transaction(get_obj());
    };

   private:
    std::set<ProbeListener*> probe_listeners{};
    std::set<CallListener*> call_listeners{};
  };

}  // namespace Gum

extern std::unique_ptr<Gum::Interceptor> g_Interceptor;
