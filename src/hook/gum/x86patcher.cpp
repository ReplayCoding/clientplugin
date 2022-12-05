#include <frida-gum.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>

#include "hook/gum/x86patcher.hpp"
#include "util/error.hpp"

X86Patcher::X86Patcher(uintptr_t address,
                       std::size_t code_size,
                       std::function<void(GumX86Writer*)> function,
                       bool enable) {
  original_code = new uint8_t[code_size];
  std::memcpy(original_code, reinterpret_cast<void*>(address), code_size);

  callback_data.address = address;
  callback_data.code_size = code_size;
  callback_data.function = function;
  callback_data.original_code = original_code;

  is_enabled = enable;
  if (is_enabled)
    this->enable();
}

void X86Patcher::enable() {
  gum_memory_patch_code(
      reinterpret_cast<void*>(callback_data.address), callback_data.code_size,
      [](void* memory, void* user_data) {
        auto callback_data = static_cast<callback_data_t*>(user_data);
        auto x86writer = gum_x86_writer_new(memory);
        x86writer->pc = callback_data->address;
        callback_data->function(x86writer);
        if ((x86writer->pc - callback_data->address) >
            callback_data->code_size) {
          // TODO: relocate the target code to automatically handle this case
          // instead of just bailing out
          throw StringError(
              "Size of patch code does not match up with target code size");
        };
        gum_x86_writer_unref(x86writer);
      },
      static_cast<void*>(&callback_data));
}

void X86Patcher::disable() {
  gum_memory_patch_code(
      reinterpret_cast<void*>(callback_data.address), callback_data.code_size,
      [](void* memory, void* user_data) {
        auto callback_data = static_cast<callback_data_t*>(user_data);
        memcpy(memory, reinterpret_cast<void*>(callback_data->original_code),
               callback_data->code_size);
      },
      static_cast<void*>(&callback_data));
}

X86Patcher::~X86Patcher() {
  if (is_enabled)
    disable();
  delete original_code;
}
