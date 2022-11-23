#include <elfio/elfio.hpp>
#include <tracy/Tracy.hpp>

#include "offsets/eh_frame.hpp"
#include "offsets/offsets.hpp"
#include "util/data_range_checker.hpp"
#include "util/data_view.hpp"
#include "util/error.hpp"

ElfModuleEhFrameParser::CieInfo ElfModuleEhFrameParser::handle_cie(
    const std::uintptr_t cie_address) {
  DataView cie_data{cie_address};

  uint32_t cie_length = cie_data.read<uint32_t>();

  // If Length contains the value 0xffffffff, then the length is
  // contained in the Extended Length field.
  if (cie_length == UINT32_MAX)
    throw StringError("TODO");

  const uint32_t cie_id = cie_data.read<uint32_t>();

  // Make sure this is a CIE
  if (cie_id != 0)
    throw StringError("CIE ID is {}, not 0 (@ {:08X}, start {:08X})", cie_id,
                      cie_data.data_ptr - sizeof(uint32_t), cie_address);

  const uint8_t cie_version = cie_data.read<uint8_t>();

  if (cie_version != 1)
    throw StringError("CIE version is {}, not 1 (@ {:08X})", cie_version,
                      cie_data.data_ptr);

  const std::string_view cie_augmentation_string = cie_data.read_string();

  /* auto cie_code_alignment_factor =  */ cie_data.read_uleb128();
  /* auto cie_data_alignment_factor =  */ cie_data.read_sleb128();

  // Really nice of them to not document this field
  /* uint8_t return_address_register =  */ cie_data.read<uint8_t>();

  // A 'z' may be present as the first character of the string. If present,
  // the Augmentation Data field shall be present.
  uint8_t fde_pointer_encoding{};
  if (cie_augmentation_string[0] == 'z') {
    // This field is only present if the Augmentation String contains the
    // character 'z'.
    /* uint64_t cie_augmentation_length =  */ cie_data.read_uleb128();

    for (const auto augmentation : cie_augmentation_string) {
      switch (augmentation) {
        case 'z': {
          // Already handled above
          break;
        }
        case 'R': {
          fde_pointer_encoding = cie_data.read<uint8_t>();
          break;
        }
        case 'L': {
          // FDE Augmentation Data isn't currently handled, so we can just
          // skip the encoding byte.
          /* uint8_t lsda_pointer_encoding =  */ cie_data.read<uint8_t>();
          break;
        }
        case 'P': {
          uint8_t pointer_encoding = cie_data.read<uint8_t>();
          // There are some unhandled applications, so just skip them as we
          // don't use the value
          if (!cie_data.read_dwarf_encoded_no_application(pointer_encoding))
            throw StringError("Expected personality routine.");
          break;
        }
        default: {
          throw StringError("Unknown augmentation '{}'", augmentation);
          break;
        }
      }
    }
  }

  return {.fde_pointer_encoding = fde_pointer_encoding};
}

std::vector<DataRange> ElfModuleEhFrameParser::handle_eh_frame(
    const std::uintptr_t start_address,
    const std::uintptr_t end_address) {
  std::vector<DataRange> function_ranges{};
  DataView fde_data{start_address};

  while (fde_data.data_ptr < end_address) {
    // A 4 byte unsigned value indicating the length in bytes of the CIE
    // structure, not including the Length field itself
    const uint32_t length = fde_data.read<uint32_t>();

    // This can happen due to section padding
    if (fde_data.data_ptr >= end_address || length == 0)
      break;

    // Do this after incrementing current_address, so we exclude the length
    // pointer.
    const auto next_record_address = fde_data.data_ptr + length;

    // If Length contains the value 0xffffffff, then the length is contained
    // in the Extended Length field.
    if (length == UINT32_MAX)
      throw StringError("TODO");

    // Used to differentiate between CIE and FDE
    // If it's nonzero, it's a value that when subtracted from the offset
    // of the CIE Pointer in the current FDE yields the offset of the
    // start of the associated CIE.
    const auto cie_id_or_cie_offset = fde_data.read<uint32_t>();

    // // If this is zero, it's a CIE, otherwise it's an FDE
    if (cie_id_or_cie_offset == 0) {
      // Skip the CIE, we parse it when handling the FDE
    } else {
      // A 4 byte unsigned value that when subtracted from the offset of the
      // CIE Pointer in the current FDE yields the offset of the start of the
      // associated CIE.
      const auto cie_pointer =
          fde_data.data_ptr - cie_id_or_cie_offset -
          sizeof(uint32_t);  // We need to subtract the sizeof because it's
                             // added when grabbing the CIE offset
      const auto cie_data = handle_cie(cie_pointer);

      std::optional<std::uintptr_t> fde_pc_begin =
          fde_data.read_dwarf_encoded(cie_data.fde_pointer_encoding);
      if (!fde_pc_begin) {
        throw StringError("PC Begin required, but not provided");
      }

      auto fde_pc_range = fde_data.read<std::uintptr_t>();
      function_ranges.push_back(
          DataRange{fde_pc_begin.value(), fde_pc_range});
    }

    // We don't parse the entire structure, but if we overflow into the next
    // record, something is wrong
    if (fde_data.data_ptr >= next_record_address) {
      throw StringError(
          "fde data pointer ({:08X}) is more than next_record_address "
          "({:08X})",
          fde_data.data_ptr, next_record_address);
    };
    fde_data.data_ptr = next_record_address;

    // Both CIEs and FDEs shall be aligned to an addressing unit sized
    // boundary.
    while ((fde_data.data_ptr % sizeof(void*)) != 0)
      fde_data.data_ptr += 1;
  }

  return function_ranges;
}

ElfModuleEhFrameParser::ElfModuleEhFrameParser(LoadedModule* module) {
  ZoneScoped;
  base_address = module->base_address;

  // TODO: Range
  for (ELFIO::section* section : module->elf.sections) {
    if (section->get_name() == ".eh_frame") {
      auto eh_frame_address =
          module->get_online_address_from_offline(section->get_address());
      std::uintptr_t eh_frame_end_addr = eh_frame_address + section->get_size();

      function_ranges = handle_eh_frame(eh_frame_address, eh_frame_end_addr);
    }
  }
}
