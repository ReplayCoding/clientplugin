#pragma once

#include <cstdint>
#include <cstring>
#include <string_view>

#include "util/error.hpp"
#include "util/leb128.h"

class DataView {
 public:
  DataView(const uintptr_t data_ptr) : _data_ptr(data_ptr){};

  inline void set_data_pointer(const uintptr_t data_ptr) {
    _data_ptr = data_ptr;
  }
  inline std::uintptr_t data_pointer() { return _data_ptr; };

  inline uint32_t read_u32() { return read_data<uint32_t>(); }
  inline uint8_t read_u8() { return read_data<uint8_t>(); }

  inline std::string_view read_string() {
    const char* string_raw = reinterpret_cast<char*>(_data_ptr);
    _data_ptr += strlen(string_raw) + 1 /* null byte */;

    return string_raw;
  }

  inline uint64_t read_uleb128() {
    const char* leb_error{};
    unsigned leb_size{};

    auto decoded_leb = decodeULEB128(reinterpret_cast<uint8_t*>(_data_ptr),
                                     &leb_size, nullptr, &leb_error);
    if (leb_error != nullptr)
      throw StringError("Error while decoding unsigned LEB128 at {:08X}: {}",
                        _data_ptr, leb_error);
    _data_ptr += leb_size;

    return decoded_leb;
  }

  inline int64_t read_sleb128() {
    const char* leb_error{};
    unsigned leb_size{};

    auto decoded_leb = decodeSLEB128(reinterpret_cast<uint8_t*>(_data_ptr),
                                     &leb_size, nullptr, &leb_error);
    if (leb_error != nullptr)
      throw StringError("Error while decoding signed LEB128 at {:08X}: {}",
                        _data_ptr, leb_error);
    _data_ptr += leb_size;

    return decoded_leb;
  }

 private:
  template <typename T>
  inline T read_data() {
    T data = *(reinterpret_cast<T*>(_data_ptr));
    _data_ptr += sizeof(T);

    return data;
  }

  std::uintptr_t _data_ptr;
};
