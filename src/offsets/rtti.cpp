#include <elf.h>
#include <fcntl.h>
#include <fmt/chrono.h>
#include <fmt/core.h>
#include <frida-gum.h>
#include <gelf.h>
#include <libelf.h>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <iterator>
#include <range/v3/all.hpp>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "rtti.hpp"
#include "util/data_range_checker.hpp"
#include "util/data_view.hpp"
#include "util/error.hpp"
#include "util/mmap.hpp"

std::unique_ptr<RttiManager> g_RTTI{};

GElf_Addr ElfModuleVtableDumper::calculate_offline_baseaddr() {
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

  throw StringError("failed to find base address");
}

std::uintptr_t ElfModuleVtableDumper::get_online_address_from_offline(
    GElf_Addr offline_addr) {
  return online_baseaddr + (offline_addr - offline_baseaddr);
}

std::uintptr_t ElfModuleVtableDumper::get_offline_address_from_online(
    std::uintptr_t online_addr) {
  return offline_baseaddr + (online_addr - online_baseaddr);
}

void ElfModuleVtableDumper::get_relocations(Elf_Scn* scn, GElf_Shdr* shdr) {
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
        throw StringError("failed to get symbol index for reloc: {}", relidx);

      auto _symbol_name =
          elf_strptr(elf, symbol_section_header.sh_link, symbol.st_name);
      if (_symbol_name == nullptr)
        throw StringError("failed to get symbol name");

      // Clone this so we can store it
      std::string symbol_name{_symbol_name};
      std::uintptr_t online_reladdress =
          get_online_address_from_offline(rel.r_offset);

      relocations[online_reladdress] = symbol_name;
    }
  }
}

ElfModuleVtableDumper::cie_info_t ElfModuleVtableDumper::handle_cie(
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
          // FDE Augmentation Data isn't currently handled, so we can just skip
          // the encoding byte.
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

DataRangeChecker ElfModuleVtableDumper::handle_eh_frame(
    const std::uintptr_t start_address,
    const std::uintptr_t end_address) {
  // Ugly name, but we need to do this to avoid conflicts
  auto function_ranges_to_return = DataRangeChecker(online_baseaddr);
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
                             // added when grabbing the CIE offset
      const auto cie_data = handle_cie(cie_pointer);

      std::optional<std::uintptr_t> fde_pc_begin =
          fde_data.read_dwarf_encoded(cie_data.fde_pointer_encoding);
      if (!fde_pc_begin) {
        throw StringError("PC Begin required, but not provided");
      }

      std::uintptr_t fde_pc_range = fde_data.read<std::uintptr_t>();
      function_ranges_to_return.add_range(fde_pc_begin.value(), fde_pc_range);
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

  return function_ranges_to_return;
}

void ElfModuleVtableDumper::generate_data_from_sections() {
  // if the section is null, we read the first section
  Elf_Scn* cur_scn = nullptr;
  DataRangeChecker section_ranges_{online_baseaddr};

  while ((cur_scn = elf_nextscn(elf, cur_scn)) != nullptr) {
    GElf_Shdr hdr{};

    if (gelf_getshdr(cur_scn, &hdr) == nullptr)
      throw StringError("Error getting section {}", elf_ndxscn(cur_scn));

    std::string_view section_name =
        elf_strptr(elf, string_header_index, hdr.sh_name);

    if (hdr.sh_addr != 0)
      section_ranges_.add_range(get_online_address_from_offline(hdr.sh_addr),
                                hdr.sh_size);

    if (hdr.sh_type == SHT_REL) {
      get_relocations(cur_scn, &hdr);
    }

    if (section_name == ".eh_frame") {
      auto eh_frame_address = get_online_address_from_offline(hdr.sh_addr);
      std::uintptr_t eh_frame_end_addr = eh_frame_address + hdr.sh_size;

      function_ranges = handle_eh_frame(eh_frame_address, eh_frame_end_addr);
    }
  }

  section_ranges = std::move(section_ranges_);
}

size_t ElfModuleVtableDumper::get_typeinfo_size(std::uintptr_t addr) {
  const auto start_addr = addr;
  const auto vtable_type = relocations[addr];
  addr += 2 * sizeof(void*); /* vtable ptr + name ptr */

  if (vtable_type.ends_with("__si_class_type_infoE")) {
    addr += sizeof(void*);
  } else if (vtable_type.ends_with("__vmi_class_type_infoE")) {
    addr += sizeof(unsigned int);  // __flags

    auto base_count = *reinterpret_cast<unsigned int*>(addr);
    addr += sizeof(unsigned int);

    for (unsigned int base{0}; base < base_count; base++) {
      addr += sizeof(void*);
      addr += sizeof(long);
    }
  } else if (vtable_type.ends_with("__class_type_infoE")) {
    // already handled
  } else {
    throw StringError("Unkown RTTI type {}", vtable_type);
  }

  return addr - start_addr;
}

std::uintptr_t get_typeinfo_addr(std::uintptr_t v) {
  return *reinterpret_cast<uintptr_t*>(v - sizeof(void*));
}

std::vector<std::uintptr_t> ElfModuleVtableDumper::locate_vftables() {
  std::unordered_set<std::uintptr_t> instances_of_typeinfo_relocs{};
  for (const auto& [addr, name] : relocations) {
    if (name.ends_with("_class_type_infoE"))
      instances_of_typeinfo_relocs.insert(addr);
  }

  std::vector<std::uintptr_t> vftable_candidates_rtti_ptr_with_cvtables{};
  DataRangeChecker typeinfo_ranges{online_baseaddr};

  for (std::uintptr_t addr{online_baseaddr};
       addr < (online_baseaddr + online_size); addr += sizeof(void*)) {
    // Is this address even mapped? If not, we would get a segfault
    if (!section_ranges.is_position_in_range(addr))
      continue;

    uintptr_t potential_typeinfo_addr = *reinterpret_cast<uintptr_t*>(addr);
    if (!function_ranges.is_position_in_range(addr) &&
        instances_of_typeinfo_relocs.contains(potential_typeinfo_addr)) {
      auto typeinfo_size = get_typeinfo_size(potential_typeinfo_addr);

      vftable_candidates_rtti_ptr_with_cvtables.push_back(addr);
      typeinfo_ranges.add_range(potential_typeinfo_addr, typeinfo_size);
    }
  }

  vftable_candidates_rtti_ptr_with_cvtables |=
      ranges::actions::remove_if([&typeinfo_ranges](auto candidate) {
        // This is a reference inside of a typeinfo graph, not a vtable
        return typeinfo_ranges.is_position_in_range(candidate);
      });

  ranges::sort(vftable_candidates_rtti_ptr_with_cvtables);

  // Now actually make this a list of vftables
  std::vector<std::uintptr_t> vftable_candidates =
      std::move(vftable_candidates_rtti_ptr_with_cvtables);
  ranges::for_each(vftable_candidates,
                   [](std::uintptr_t& v) { v += sizeof(void*); });

  // Generate a list of constructor vtables, this unfortunately means we
  // lose some real vtables
  std::unordered_set<std::uintptr_t> invalid_typeinfo{};
  std::unordered_set<std::uintptr_t> seen_typeinfo{};
  std::uintptr_t prev_typeinfo{0};

  for (auto& candidate : vftable_candidates) {
    uintptr_t typeinfo_addr = get_typeinfo_addr(candidate);

    // Avoid constructor vftables while keeping normal consecutive subtables
    if ((typeinfo_addr != prev_typeinfo) &&
        seen_typeinfo.contains(typeinfo_addr)) {
      invalid_typeinfo.insert(typeinfo_addr);
    }

    prev_typeinfo = typeinfo_addr;
    seen_typeinfo.insert(typeinfo_addr);
  }

  vftable_candidates |=
      ranges::actions::remove_if([&invalid_typeinfo](auto candidate) {
        uintptr_t typeinfo_addr = get_typeinfo_addr(candidate);
        return invalid_typeinfo.contains(typeinfo_addr);
      });

  return vftable_candidates;
}

ElfModuleVtableDumper::vtables_t ElfModuleVtableDumper::get_vtables() {
  auto vf_tables = locate_vftables();

  ElfModuleVtableDumper::vtables_t vtables{};
  for (const auto& vtable_rng :
       vf_tables |
           ranges::views::chunk_by([](std::uintptr_t a, std::uintptr_t b) {
             return get_typeinfo_addr(a) == get_typeinfo_addr(b);
           })) {
    auto vtable = ranges::to<std::vector>(vtable_rng);
    std::string typeinfo_name =
        *reinterpret_cast<char**>(get_typeinfo_addr(vtable[0]) + sizeof(void*));
    vtables[typeinfo_name] = vtable;
  };

  return vtables;
}

ElfModuleVtableDumper::ElfModuleVtableDumper(const std::string path,
                                             size_t baddr,
                                             size_t size) {
  try {
    mapped_file = std::make_unique<MemoryMappedFile>(path);
  } catch (std::exception& e) {
    throw StringError("WARNING: failed to load module {} ({})", path, e.what());
  }

  elf = elf_memory(static_cast<char*>(mapped_file->address()),
                   mapped_file->length());
  if (elf == nullptr) {
    throw StringError("note: failed to load module: {}", path);
  };

  if (elf_getshdrstrndx(elf, &string_header_index) < 0) {
    throw StringError("failed to get sh string section");
  };

  // Find online & offline load addresses
  online_baseaddr = baddr;
  online_size = size;
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

  generate_data_from_sections();
}

std::uintptr_t RttiManager::get_function(std::string module,
                                         std::string name,
                                         uint16_t vftable,
                                         uint16_t function) {
  auto vftable_ptr = module_vtables[module][name][vftable];
  return *reinterpret_cast<std::uintptr_t*>(vftable_ptr +
                                            (sizeof(void*) * function));
}

RttiManager::RttiManager() {
  elf_version(EV_CURRENT);
  auto start_time = std::chrono::high_resolution_clock::now();

  gum_process_enumerate_modules(
      // This uses a gboolean, so we need to return `1` instead of `true`.
      [](const GumModuleDetails* details, void* user_data) {
        auto self = static_cast<RttiManager*>(user_data);
        // ignore vdso
        constexpr std::string_view excluded_paths[] = {"linux-vdso.so.1",
                                                       "libclientplugin.so"};
        if (ranges::any_of(excluded_paths,
                           [&](auto b) { return b == details->name; })) {
          return 1;
        }

        try {
          ElfModuleVtableDumper dumped_rtti{
              details->path, static_cast<size_t>(details->range->base_address),
              details->range->size};

          auto vtables = dumped_rtti.get_vtables();
          self->submit_vtables(details->name, vtables);
        } catch (std::exception& e) {
          fmt::print(
              "while handling module {}:\n"
              "\t{}\n",
              details->name, e.what());
        };

        return 1;
      },
      this);

  auto duration = std::chrono::high_resolution_clock::now() - start_time;
  fmt::print("Handling modules took {}\n",
             std::chrono::duration_cast<std::chrono::milliseconds>(duration));
}
