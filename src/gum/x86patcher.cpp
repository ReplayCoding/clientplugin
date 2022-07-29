#include <cstddef>
#include <cstdint>
#include <cstring>
#include <frida-gum.h>
#include <functional>
#include <gum/x86patcher.hpp>
#include <memory>

X86Patcher::X86Patcher(std::uintptr_t address, std::size_t size,
                       std::function<void(GumX86Writer *)> function) {
  original_code = new uint8_t[size];
  memcpy(original_code, reinterpret_cast<void *>(address), size);

  cons_callback_data.address = address;
  cons_callback_data.size = size;
  cons_callback_data.function = function;

  des_callback_data.address = reinterpret_cast<void *>(address),
  des_callback_data.size = size,
  des_callback_data.original_code = original_code,

  gum_memory_patch_code(
      reinterpret_cast<void *>(address), size,
      [](void *memory, void *user_data) {
        auto callback_data = static_cast<cons_callback_data_t *>(user_data);
        auto x86writer = gum_x86_writer_new(memory);
        x86writer->pc = callback_data->address;
        callback_data->function(x86writer);
        gum_x86_writer_unref(x86writer);
      },
      static_cast<void *>(&cons_callback_data));
};

X86Patcher::~X86Patcher() {
  gum_memory_patch_code(
      des_callback_data.address, des_callback_data.size,
      [](void *memory, void *user_data) {
        auto callback_data = static_cast<des_callback_data_t *>(user_data);
        memcpy(memory, callback_data->address, callback_data->size);
      },
      static_cast<void *>(&des_callback_data));
  delete original_code;
};
