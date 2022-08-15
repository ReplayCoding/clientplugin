#pragma once

#include <frida-gum.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>

class X86Patcher {
 public:
  X86Patcher(std::uintptr_t address,
             std::size_t size,
             std::function<void(GumX86Writer*)> function);
  ~X86Patcher();

 private:
  struct callback_data_t {
    std::uintptr_t address;
    std::size_t code_size;
    std::function<void(GumX86Writer*)> function;
    uint8_t* original_code;
  } callback_data{};

  uint8_t* original_code;
};
