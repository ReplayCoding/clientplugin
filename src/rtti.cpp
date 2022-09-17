#include <elf.h>
#include <fcntl.h>
#include <fmt/chrono.h>
#include <fmt/core.h>
#include <frida-gum.h>
#include <gelf.h>
#include <libelf.h>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <string_view>

#include "rtti.hpp"
#include "util/data_view.hpp"
#include "util/error.hpp"
#include "util/mmap.hpp"

GElf_Addr ElfModuleRttiDumper::calculate_offline_baseaddr() {
  GElf_Ehdr ehdr;
  gelf_getehdr(elf, &ehdr);

  auto phdr_count = ehdr.e_phnum;

  for (auto idx = 0; idx != phdr_count; idx++) {
    GElf_Phdr phdr;
    gelf_getphdr(elf, idx, &phdr);

    if (phdr.p_type == PT_LOAD && phdr.p_offset == 0) {
      return phdr.p_vaddr;
    }
  }

  // We failed to find a base address for this binary
  throw StringError("Failed to find base address");
}

std::uintptr_t ElfModuleRttiDumper::get_online_address_from_offline(
    GElf_Addr offline_addr) {
  return online_baseaddr + (offline_addr - offline_baseaddr);
}

void ElfModuleRttiDumper::handle_relocations(Elf_Scn* scn, GElf_Shdr* shdr) {
  // This really really looks like a memory leak :(
  Elf_Data* section_data = elf_getdata(scn, nullptr);
  if (section_data == nullptr) {
    throw StringError("failed to get data for section {}: {}", shdr->sh_name);
  };

  auto reloc_struct_size = gelf_fsize(elf, ELF_T_REL, 1, EV_CURRENT);
  auto number_of_relocs = shdr->sh_size / reloc_struct_size;

  // really well designed api we have here
  Elf_Scn* symbol_section;
  if ((symbol_section = elf_getscn(elf, shdr->sh_link)) == nullptr)
    throw StringError("failed to get sh_link section");

  GElf_Shdr symbol_section_header;
  if (gelf_getshdr(symbol_section, &symbol_section_header) == nullptr)
    throw StringError("failed to get sh_link section header");

  Elf_Data* symbol_section_data = elf_getdata(symbol_section, nullptr);

  for (size_t relidx = 0; relidx < number_of_relocs; ++relidx) {
    GElf_Rel rel;
    if (gelf_getrel(section_data, relidx, &rel) == nullptr) {
      throw StringError("couldn't get relocation for section {}: {}",
                        elf_strptr(elf, string_header_index, shdr->sh_name),
                        relidx);
    }

    if (GELF_R_TYPE(rel.r_info) != 0 /* NONE rel type */ &&
        GELF_R_SYM(rel.r_info) != STN_UNDEF /* no symbol */) {
      GElf_Sym symbol;
      if (gelf_getsymshndx(symbol_section_data, nullptr, GELF_R_SYM(rel.r_info),
                           &symbol, nullptr) == nullptr)
        throw StringError("Failed to get symbol index for reloc: {}", relidx);

      auto _symbol_name =
          elf_strptr(elf, symbol_section_header.sh_link, symbol.st_name);
      if (_symbol_name == nullptr)
        throw StringError("Failed to get symbol name");

      // Clone this so we can store it
      std::string symbol_name{_symbol_name};
      std::uintptr_t online_reladdress =
          get_online_address_from_offline(rel.r_offset);

      _relocations[online_reladdress] = symbol_name;
    }
  }
}

ElfModuleRttiDumper::cie_info_t ElfModuleRttiDumper::handle_cie(
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

  // I want to fucking strangle whoever put this field in but didn't add
  // documentation for it
  /* uint8_t return_address_register =  */ cie_data.read<uint8_t>();  // unused

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
          // FDE Augmentation Data isn't currently handled, so we can just skip
          // the encoding byte.
          /* uint8_t lsda_pointer_encoding =  */ cie_data.read<uint8_t>();
          break;
        }
        case 'P': {
          uint8_t pointer_encoding = cie_data.read<uint8_t>();
          // Theres some unhandled applications, so just skip them as we don't
          // use the value
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
void ElfModuleRttiDumper::handle_eh_frame(const std::uintptr_t start_address,
                                          const std::uintptr_t end_address) {
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

    // If Length contains the value 0xffffffff, then the length is contained in
    // the Extended Length field.
    if (length == UINT32_MAX)
      throw StringError("TODO");

    // Used to differentiate between CIE and FDE
    // If it's nonzero, it's a value that when subtracted from the offset
    // of the CIE Pointer in the current FDE yields the offset of the
    // start of the associated CIE.
    uint32_t cie_id_or_cie_offset = fde_data.read<uint32_t>();

    // // If this is zero, it's a CIE, otherwise it's an FDE
    if (cie_id_or_cie_offset == 0) {
      // Skip the CIE, we parse it when handling the FDE
    } else {
      // A 4 byte unsigned value that when subtracted from the offset of the CIE
      // Pointer in the current FDE yields the offset of the start of the
      // associated CIE.
      const auto cie_pointer =
          fde_data.data_ptr - cie_id_or_cie_offset -
          sizeof(uint32_t);  // We need to subtract the sizeof because it's
                             // added when grabbing the offset
      const auto cie_data = handle_cie(cie_pointer);

      if (auto fde_pointer =
              fde_data.read_dwarf_encoded(cie_data.fde_pointer_encoding)) {
        fmt::print("PCBEGIN: {:08X}\n", fde_pointer.value());
      } else {
        throw StringError("PC Begin required, but not provided");
      }

      auto fde_pc_range = fde_data.read<std::uintptr_t>();
      fmt::print("PCRANGE: {:X}\n", fde_pc_range);
    }

    // We don't parse the entire structure, but if we overflow into the next
    // record, something is wrong
    if (fde_data.data_ptr >= next_record_address) {
      throw StringError(
          "fde data pointer ({:08X}) is more than next_record_address ({:08X})",
          fde_data.data_ptr, next_record_address);
    };
    fde_data.data_ptr = next_record_address;

    // Both CIEs and FDEs shall be aligned to an addressing unit sized boundary.
    while ((fde_data.data_ptr % sizeof(void*)) != 0)
      fde_data.data_ptr += 1;
  }
}

ElfModuleRttiDumper::ElfModuleRttiDumper(const std::string path) {
  try {
    mapped_file = std::make_unique<MemoryMappedFile>(path);
  } catch (std::exception& e) {
    throw StringError("WARNING: failed to load module {} ({})", path, e.what());
  }

  elf = elf_memory(static_cast<char*>(mapped_file->address()),
                   mapped_file->length());
  // If we failed to load the file, just continue on
  if (elf == nullptr) {
    throw StringError("note: failed to load module: {}", path);
  };

  if (elf_getshdrstrndx(elf, &string_header_index) < 0) {
    throw StringError("failed to get sh string section");
  };

  // Find online & offline load addresses
  online_baseaddr = gum_module_find_base_address(path.c_str());
  try {
    offline_baseaddr = calculate_offline_baseaddr();
  } catch (std::exception& e) {
    elf_end(elf);
    throw StringError("WARNING: {}", e.what());
  }
  if (online_baseaddr == 0) {
    // bail out, this is bad
    elf_end(elf);
    throw StringError("WARNING: online_baseaddress is nullptr");
  };

  // if the section is null, we read the first section
  Elf_Scn* cur_scn = nullptr;
  while ((cur_scn = elf_nextscn(elf, cur_scn)) != nullptr) {
    GElf_Shdr hdr{};

    if (gelf_getshdr(cur_scn, &hdr) == nullptr)
      throw StringError("Error getting section {}", elf_ndxscn(cur_scn));

    std::string_view section_name =
        elf_strptr(elf, string_header_index, hdr.sh_name);

    if (hdr.sh_type == SHT_REL) {
      handle_relocations(cur_scn, &hdr);
    }

    // FIXME
    if (section_name == ".eh_frame") {
      auto eh_frame_address = get_online_address_from_offline(hdr.sh_addr);
      std::uintptr_t eh_frame_end_addr = eh_frame_address + hdr.sh_size;

      fmt::print("{}: {:08X} ({:08X}) [offline start: {:08X}]\n", path,
                 eh_frame_address, eh_frame_end_addr, hdr.sh_addr);
      handle_eh_frame(eh_frame_address, eh_frame_end_addr);
    }
  }
}

void LoadRtti() {
  elf_version(EV_CURRENT);
  auto start_time = std::chrono::high_resolution_clock::now();
  gum_process_enumerate_modules(
      [](const GumModuleDetails* details, void* user_data) {
        // ignore vdso
        constexpr std::string_view vdso_path = "linux-vdso.so.1";
        if (vdso_path == details->name)
          return 1;

        try {
          ElfModuleRttiDumper dumped_rtti{details->path};
          // for (const auto& [addr, name] : dumped_rtti.relocations()) {
          //   if (name == "_ZTVN10__cxxabiv117__class_type_infoE" ||
          //       name == "_ZTVN10__cxxabiv120__si_class_type_infoE" ||
          //       name == "_ZTVN10__cxxabiv121__vmi_class_type_infoE") {
          //     fmt::print("Found reloc: {:08X} -> {} ({})\n",
          //                addr - details->range->base_address, name,
          //                details->name);
          //   }
          // }
        } catch (std::exception& e) {
          fmt::print(
              "while handling module {}:\n"
              "\t{}\n",
              details->name, e.what());
        };
        // this is stupid
        return 1;
      },
      nullptr);
  auto duration = std::chrono::high_resolution_clock::now() - start_time;
  fmt::print("Handling modules took {}\n",
             std::chrono::duration_cast<std::chrono::milliseconds>(duration));
}
