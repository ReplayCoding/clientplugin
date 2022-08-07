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
  struct cons_callback_data_t {
    std::uintptr_t address;
    std::size_t size;
    std::function<void(GumX86Writer*)> function;
  } cons_callback_data{};
  struct des_callback_data_t {
    void* address;
    std::size_t size;
    uint8_t* original_code;
  } des_callback_data{};

  uint8_t* original_code;
};
