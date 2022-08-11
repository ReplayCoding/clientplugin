#pragma once
#include <frida-gum.h>
#include <cassert>
#include <cstdint>
#include <functional>
#include <gum/interceptor.hpp>

// POLYMORPHISM BE DAMNED

using attachment_hook_func_t = std::function<void(GumInvocationContext*)>;
// Internal utility class
class _CallAttachmentHook : private Gum::CallListener {
 public:
  _CallAttachmentHook(std::uintptr_t address) {
    g_Interceptor->attach(address, this, nullptr);
  };
  ~_CallAttachmentHook() { g_Interceptor->detach(this); };
};
class _ProbeAttachmentHook : private Gum::ProbeListener {
 public:
  _ProbeAttachmentHook(std::uintptr_t address) {
    g_Interceptor->attach(address, this, nullptr);
  };
  ~_ProbeAttachmentHook() { g_Interceptor->detach(this); };
};

class AttachmentHookEnter : public _ProbeAttachmentHook {
 public:
  AttachmentHookEnter(std::uintptr_t address, attachment_hook_func_t func)
      : _ProbeAttachmentHook(address), func(func){};

 private:
  void on_hit(GumInvocationContext* ctx) override { func(ctx); };
  attachment_hook_func_t func;
};

class AttachmentHookLeave : public _CallAttachmentHook {
 public:
  AttachmentHookLeave(std::uintptr_t address, attachment_hook_func_t func)
      : _CallAttachmentHook(address), func(func){};

 private:
  void on_enter(GumInvocationContext*) override{};
  void on_leave(GumInvocationContext* ctx) override { func(ctx); };
  attachment_hook_func_t func;
};

class AttachmentHookBoth : public _CallAttachmentHook {
 public:
  AttachmentHookBoth(std::uintptr_t address,
                     attachment_hook_func_t enter_func,
                     attachment_hook_func_t leave_func)
      : _CallAttachmentHook(address),
        enter_func(enter_func),
        leave_func(leave_func){};

 private:
  void on_enter(GumInvocationContext* ctx) override { enter_func(ctx); };
  void on_leave(GumInvocationContext* ctx) override { leave_func(ctx); };
  attachment_hook_func_t enter_func;
  attachment_hook_func_t leave_func;
};
