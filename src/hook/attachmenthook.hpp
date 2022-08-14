#pragma once
#include <frida-gum.h>
#include <cassert>
#include <cstdint>
#include <functional>

#include "hook/gum/interceptor.hpp"
#include "offsets.hpp"

// Wrapper for GumInvocationContext
class InvocationContext {
 public:
  InvocationContext(GumInvocationContext* context) : context(context) {}

  template <typename T>
  inline T get_arg(size_t index) {
    return reinterpret_cast<T>(
        gum_invocation_context_get_nth_argument(context, index));
  }

  template <typename T>
  inline void set_arg(size_t index, T value) {
    gum_invocation_context_replace_nth_argument(context, index,
                                                reinterpret_cast<void*>(value));
  }

  template <typename T>
  inline T get_return() {
    return reinterpret_cast<T>(
        gum_invocation_context_get_return_value(context));
  }

  template <typename T>
  inline void set_return(T value) {
    gum_invocation_context_replace_return_value(context,
                                                reinterpret_cast<void*>(value));
  }

 private:
  GumInvocationContext* context;
};

// POLYMORPHISM BE DAMNED

using attachment_hook_func_t = std::function<void(InvocationContext)>;
// Internal utility class
class _CallAttachmentHook : private Gum::CallListener {
 public:
  _CallAttachmentHook(const offsets::Offset& address) {
    g_Interceptor->attach(address, this, nullptr);
  }
  ~_CallAttachmentHook() { g_Interceptor->detach(this); }
};
class _ProbeAttachmentHook : private Gum::ProbeListener {
 public:
  _ProbeAttachmentHook(const offsets::Offset& address) {
    g_Interceptor->attach(address, this, nullptr);
  }
  ~_ProbeAttachmentHook() { g_Interceptor->detach(this); }
};

class AttachmentHookEnter : public _ProbeAttachmentHook {
 public:
  AttachmentHookEnter(const offsets::Offset& address,
                      attachment_hook_func_t func)
      : _ProbeAttachmentHook(address), func(func) {}

 private:
  void on_hit(GumInvocationContext* ctx) override {
    func(InvocationContext(ctx));
  }
  attachment_hook_func_t func;
};

class AttachmentHookLeave : public _CallAttachmentHook {
 public:
  AttachmentHookLeave(const offsets::Offset& address,
                      attachment_hook_func_t func)
      : _CallAttachmentHook(address), func(func) {}

 private:
  void on_enter(GumInvocationContext*) override {}
  void on_leave(GumInvocationContext* ctx) override {
    func(InvocationContext(ctx));
  }
  attachment_hook_func_t func;
};

class AttachmentHookBoth : public _CallAttachmentHook {
 public:
  AttachmentHookBoth(offsets::Offset& address,
                     attachment_hook_func_t enter_func,
                     attachment_hook_func_t leave_func)
      : _CallAttachmentHook(address),
        enter_func(enter_func),
        leave_func(leave_func) {}

 private:
  void on_enter(GumInvocationContext* ctx) override {
    enter_func(InvocationContext(ctx));
  }
  void on_leave(GumInvocationContext* ctx) override {
    leave_func(InvocationContext(ctx));
  }
  attachment_hook_func_t enter_func;
  attachment_hook_func_t leave_func;
};
