#pragma once

#include <frida-gum.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>

class X86Patcher {
 public:
  X86Patcher(uintptr_t address,
             std::size_t size,
             std::function<void(GumX86Writer*)> function,
             bool enable = true);
  ~X86Patcher();

  void enable();
  void disable();

 private:
  struct callback_data_t {
    uintptr_t address;
    std::size_t code_size;
    std::function<void(GumX86Writer*)> function;
    uint8_t* original_code;
  } callback_data{};

  uint8_t* original_code;
  bool is_enabled;
};
