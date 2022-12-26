#pragma once
#include <bit>
#include <frida-gum.h>
#include <cstdint>
#include <functional>

#include "hook/gum/interceptor.hpp"
#include "offsets/offsets.hpp"

// Wrapper for GumInvocationContext
class InvocationContext {
 public:
  InvocationContext(GumInvocationContext* context) : context(context) {}

  template <typename T>
  inline T get_arg(size_t index) {
    return std::bit_cast<T>(
        gum_invocation_context_get_nth_argument(context, index));
  }

  template <typename T>
  inline void set_arg(size_t index, T value) {
    gum_invocation_context_replace_nth_argument(context, index,
                                                std::bit_cast<void*>(value));
  }

  template <typename T>
  inline T get_return() {
    return std::bit_cast<T>(
        gum_invocation_context_get_return_value(context));
  }

  template <typename T>
  inline void set_return(T value) {
    gum_invocation_context_replace_return_value(context,
                                                std::bit_cast<void*>(value));
  }

  inline uint32_t thread_id() {
    return gum_invocation_context_get_thread_id(context);
  }

 private:
  GumInvocationContext* context;
};

using attachment_hook_func_t = std::function<void(InvocationContext)>;

// Internal utility class
class _CallAttachmentHook : private Gum::CallListener {
 public:
  _CallAttachmentHook(const Offset& address) {
    g_Interceptor->attach(address, this, nullptr);
  }

  ~_CallAttachmentHook() { g_Interceptor->detach(this); }
};

class _ProbeAttachmentHook : private Gum::ProbeListener {
 public:
  _ProbeAttachmentHook(const Offset& address) {
    g_Interceptor->attach(address, this, nullptr);
  }

  ~_ProbeAttachmentHook() { g_Interceptor->detach(this); }
};

class AttachmentHookEnter : public _ProbeAttachmentHook {
 public:
  AttachmentHookEnter(const Offset& address, attachment_hook_func_t func)
      : _ProbeAttachmentHook(address), func(func) {}

 private:
  void on_hit(GumInvocationContext* ctx) override {
    func(InvocationContext(ctx));
  }

  attachment_hook_func_t func;
};

class AttachmentHookLeave : public _CallAttachmentHook {
 public:
  AttachmentHookLeave(const Offset& address, attachment_hook_func_t func)
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
  AttachmentHookBoth(const Offset& address,
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
