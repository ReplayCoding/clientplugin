#pragma once

#include <absl/numeric/int128.h>
#include <bit>
#include <cstdint>
#include <cstring>
#include <optional>
#include <string_view>

#include "util/error.hpp"
#include "util/leb128.hpp"

// FIXME: I don't really know where to put this, but I know for a fact that it
// shouldn't be here
enum class eh_dwarf_format {
  DW_EH_PE_absptr = 0x00,
  DW_EH_PE_uleb128 = 0x01,
  DW_EH_PE_udata2 = 0x02,
  DW_EH_PE_udata4 = 0x03,
  DW_EH_PE_udata8 = 0x04,
  DW_EH_PE_sleb128 = 0x09,
  DW_EH_PE_sdata2 = 0x0A,
  DW_EH_PE_sdata4 = 0x0B,
  DW_EH_PE_sdata8 = 0x0C,
};

enum class eh_dwarf_application {
  DW_EH_PE_pcrel = 0x10,
  DW_EH_PE_textrel = 0x20,
  DW_EH_PE_datarel = 0x30,
  DW_EH_PE_funcrel = 0x40,
  DW_EH_PE_aligned = 0x50,
  DW_EH_PE_indirect = 0x80,
};

constexpr uint8_t DW_EH_PE_omit = 0xFF;

class DataView {
 public:
  DataView(const uintptr_t _data_ptr) : data_ptr(_data_ptr){};

  template <typename T>
  inline T read() {
    T data = *(std::bit_cast<T*>(data_ptr));
    data_ptr += sizeof(T);

    return data;
  }

  inline std::string_view read_string() {
    const char* string_raw = std::bit_cast<char*>(data_ptr);
    data_ptr += strlen(string_raw) + 1 /* null byte */;

    return string_raw;
  }

  inline uint64_t read_uleb128() {
    const char* leb_error{};
    unsigned leb_size{};

    auto decoded_leb = decodeULEB128(std::bit_cast<uint8_t*>(data_ptr),
                                     &leb_size, nullptr, &leb_error);
    if (leb_error != nullptr)
      throw StringError("Error while decoding unsigned LEB128 at {:08X}: {}",
                        data_ptr, leb_error);
    data_ptr += leb_size;

    return decoded_leb;
  }

  inline int64_t read_sleb128() {
    const char* leb_error{};
    unsigned leb_size{};

    auto decoded_leb = decodeSLEB128(std::bit_cast<uint8_t*>(data_ptr),
                                     &leb_size, nullptr, &leb_error);
    if (leb_error != nullptr)
      throw StringError("Error while decoding signed LEB128 at {:08X}: {}",
                        data_ptr, leb_error);
    data_ptr += leb_size;

    return decoded_leb;
  }

  std::optional<absl::int128> read_dwarf_encoded_no_application(
      const uint8_t encoding) {
    if (encoding == DW_EH_PE_omit)
      return std::nullopt;

    const auto value_format = static_cast<eh_dwarf_format>(encoding & 0x0F);

    absl::int128 value_without_application_applied{};

    switch (value_format) {
      case eh_dwarf_format::DW_EH_PE_absptr: {
        value_without_application_applied = read<uintptr_t>();
        break;
      }

      case eh_dwarf_format::DW_EH_PE_uleb128: {
        value_without_application_applied = read_uleb128();
        break;
      }
      case eh_dwarf_format::DW_EH_PE_udata2: {
        value_without_application_applied = read<uint16_t>();
        break;
      }
      case eh_dwarf_format::DW_EH_PE_udata4: {
        value_without_application_applied = read<uint32_t>();
        break;
      }
      case eh_dwarf_format::DW_EH_PE_udata8: {
        value_without_application_applied = read<uint64_t>();
        break;
      }

      case eh_dwarf_format::DW_EH_PE_sleb128: {
        value_without_application_applied = read_sleb128();
        break;
      }
      case eh_dwarf_format::DW_EH_PE_sdata2: {
        value_without_application_applied = read<int16_t>();
        break;
      }
      case eh_dwarf_format::DW_EH_PE_sdata4: {
        value_without_application_applied = read<int32_t>();
        break;
      }
      case eh_dwarf_format::DW_EH_PE_sdata8: {
        value_without_application_applied = read<int64_t>();
        break;
      }

      default: {
        throw StringError("Unknown DWARF encoding value format: {:X}",
                          static_cast<uint8_t>(value_format));
        break;
      }
    }

    return value_without_application_applied;
  }

  std::optional<uintptr_t> read_dwarf_encoded(const uint8_t encoding) {
    const auto pc_rel_ptr = data_ptr;

    if (auto value_without_application_applied =
            read_dwarf_encoded_no_application(encoding)) {
      const auto value_application =
          static_cast<eh_dwarf_application>(encoding & 0xF0);

      absl::uint128 value_with_application{};
      switch (value_application) {
        case eh_dwarf_application::DW_EH_PE_pcrel: {
          value_with_application =
              pc_rel_ptr + value_without_application_applied.value();
          break;
        }

        default: {
          throw StringError("Unknown DWARF encoding value application: {:X}",
                            static_cast<uint8_t>(value_application));
          break;
        }
      }

      return static_cast<uintptr_t>(value_with_application);
    } else {
      return std::nullopt;
    }
  }

  uintptr_t data_ptr;
};
