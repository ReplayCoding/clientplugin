#include <fcntl.h>
#include <fmt/chrono.h>
#include <fmt/core.h>
#include <marl/defer.h>
#include <marl/waitgroup.h>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <coroutine>
#include <cstdint>
#include <cstring>
#include <elfio/elfio.hpp>
#include <iterator>
#include <mutex>
#include <range/v3/action/remove_if.hpp>
#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/algorithm/sort.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view/chunk_by.hpp>
#include <string_view>
#include <tracy/Tracy.hpp>
#include <utility>
#include <vector>

// It's just a header, it can't hurt you
// (Must be AFTER elfio)
#include <link.h>

#include "offsets/eh_frame.hpp"
#include "offsets/offsets.hpp"
#include "offsets/rtti.hpp"
#include "util/data_range_checker.hpp"
#include "util/data_view.hpp"
#include "util/error.hpp"
#include "util/generator.hpp"
#include "util/mmap.hpp"

std::unique_ptr<RttiManager> g_RTTI{};

void ElfModuleVtableDumper::get_relocations(ELFIO::section* section) {
  const ELFIO::relocation_section_accessor relocations{loaded_mod->elf,
                                                       section};
  const ELFIO::symbol_section_accessor symbols{
      loaded_mod->elf, loaded_mod->elf.sections[section->get_link()]};

  // TODO: Range
  for (size_t relidx = 0; relidx < relocations.get_entries_num(); ++relidx) {
    ELFIO::Elf64_Addr offset{};
    ELFIO::Elf_Word symbol{};
    uint8_t type{};
    ELFIO::Elf_Sxword _addend{};

    if (!relocations.get_entry(relidx, offset, symbol, type, _addend))
      throw StringError("Failed to get reloc for index {} in section {}",
                        relidx, section->get_name());

    std::string symbol_name;
    ELFIO::Elf64_Addr symbol_value;
    ELFIO::Elf_Xword _symbol_size;
    uint8_t _symbol_bind;
    uint8_t symbol_type;
    ELFIO::Elf_Half _symbol_section;
    uint8_t _symbol_other;

    if (!symbols.get_symbol(symbol, symbol_name, symbol_value, _symbol_size,
                            _symbol_bind, symbol_type, _symbol_section,
                            _symbol_other))
      continue;

    if (type == 0 /* NONE rel type */ || symbol_type == /* ELFIO:: */ STN_UNDEF)
      continue;

    std::uintptr_t online_reladdress =
        loaded_mod->get_online_address_from_offline(offset);

    // Explicitely avoid setting the relocations var defined above...
    this->relocations[online_reladdress] = symbol_name;
  }
}

void ElfModuleVtableDumper::generate_data_from_sections() {
  for (auto section : loaded_mod->elf.sections) {
    if (section->get_type() == /* ELFIO:: */ SHT_REL) {
      get_relocations(section);
      continue;
    }

    if (section->get_address() != 0) {
      section_ranges.emplace_back(
          loaded_mod->get_online_address_from_offline(section->get_address()),
          section->get_size());
    }
  }
}

size_t ElfModuleVtableDumper::get_typeinfo_size(std::uintptr_t addr) {
  const auto vtable_type = relocations[addr];
  uint32_t size = 2 * sizeof(void*); /* vtable ptr + name ptr */

  if (vtable_type.ends_with("__si_class_type_infoE")) {
    size += sizeof(void*);
  } else if (vtable_type.ends_with("__vmi_class_type_infoE")) {
    size += sizeof(unsigned int);  // __flags

    auto base_count = *reinterpret_cast<unsigned int*>(addr + size);
    size += sizeof(unsigned int);

    for (unsigned int base{0}; base < base_count; base++) {
      size += sizeof(void*);
      size += sizeof(long);
    }
  } else if (vtable_type.ends_with("__class_type_infoE")) {
    // already handled
  } else {
    throw StringError("Unkown RTTI type {}", vtable_type);
  }

  return size;
}

std::uintptr_t get_typeinfo_addr(std::uintptr_t v) {
  return *reinterpret_cast<uintptr_t*>(v - sizeof(void*));
}

Generator<std::uintptr_t> ElfModuleVtableDumper::locate_vftables() {
  std::unordered_set<std::uintptr_t> instances_of_typeinfo_relocs{};
  for (const auto& [addr, name] : relocations) {
    if (name.ends_with("_class_type_infoE"))
      instances_of_typeinfo_relocs.insert(addr);
  }

  std::vector<std::uintptr_t> vftable_candidates_rtti_ptr_with_cvtables{};
  DataRangeChecker typeinfo_ranges{loaded_mod->base_address};

  for (DataRange& range : section_ranges) {
    for (std::uintptr_t addr{range.begin}; addr < (range.begin + range.length);
         addr += sizeof(void*)) {
      uintptr_t potential_typeinfo_addr = *reinterpret_cast<uintptr_t*>(addr);
      if (!function_ranges.is_position_in_range(addr) &&
          instances_of_typeinfo_relocs.contains(potential_typeinfo_addr)) {
        auto typeinfo_size = get_typeinfo_size(potential_typeinfo_addr);

        vftable_candidates_rtti_ptr_with_cvtables.push_back(addr);
        typeinfo_ranges.add_range(
            DataRange(potential_typeinfo_addr, typeinfo_size));
      }
    }
  }

  vftable_candidates_rtti_ptr_with_cvtables |=
      ranges::actions::remove_if([&typeinfo_ranges](auto candidate) {
        // This is a reference inside of a typeinfo graph, not a vtable
        return typeinfo_ranges.is_position_in_range(candidate);
      });

  ranges::sort(vftable_candidates_rtti_ptr_with_cvtables);

  // Now actually make this a list of vftables
  auto vftable_candidates =
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
      // std::string typeinfo_name =
      //     *reinterpret_cast<char**>(typeinfo_addr + sizeof(void*));
      // fmt::print("INVALID TYPEINFO: {} @ candidate {:08X}\n", typeinfo_name,
      //            candidate);
      invalid_typeinfo.insert(typeinfo_addr);
    }

    prev_typeinfo = typeinfo_addr;
    seen_typeinfo.insert(typeinfo_addr);
  }

  for (auto& candidate : vftable_candidates) {
    uintptr_t typeinfo_addr = get_typeinfo_addr(candidate);
    if (invalid_typeinfo.contains(typeinfo_addr))
      continue;

    co_yield candidate;
  }
}

Generator<ElfModuleVtableDumper::Vtable> ElfModuleVtableDumper::get_vtables() {
  ZoneScoped;
  std::vector<std::uintptr_t> vf_tables;
  auto i = locate_vftables();
  // FIXME!
  std::copy(i.begin(), i.end(), std::back_inserter(vf_tables));

  for (const auto& vtable_rng :
       vf_tables |
           ranges::views::chunk_by([](std::uintptr_t a, std::uintptr_t b) {
             return get_typeinfo_addr(a) == get_typeinfo_addr(b);
           })) {
    auto vtable = ranges::to<std::vector>(vtable_rng);
    std::string typeinfo_name =
        *reinterpret_cast<char**>(get_typeinfo_addr(vtable[0]) + sizeof(void*));
    co_yield ElfModuleVtableDumper::Vtable{typeinfo_name, vtable};
  };
}

ElfModuleVtableDumper::ElfModuleVtableDumper(LoadedModule* module,
                                             ElfModuleEhFrameParser* eh_frame) {
  ZoneScoped;
  loaded_mod = module;
  function_ranges = std::move(eh_frame->as_data_range_checker());

  generate_data_from_sections();
}

std::uintptr_t RttiManager::get_function(std::string module,
                                         std::string name,
                                         uint16_t vftable,
                                         uint16_t function) {
  auto vftable_ptr = module_vtables.at(std::pair{module, name}).at(vftable);
  return *reinterpret_cast<std::uintptr_t*>(vftable_ptr +
                                            (sizeof(void*) * function));
}

struct module_info {
  std::string name;
  std::uintptr_t addr;
};

RttiManager::RttiManager() {
  auto start_time = std::chrono::high_resolution_clock::now();
  std::vector<module_info> modules;

  dl_iterate_phdr(
      [](dl_phdr_info* info, size_t info_size, void* user_data) {
        ZoneScopedN("dl_iterate_phdr func");

        auto* modules = reinterpret_cast<std::vector<module_info>*>(user_data);
        const std::string_view fname = basename(info->dlpi_name);

        constexpr std::string_view excluded_paths[] = {"linux-vdso.so.1",
                                                       "libclientplugin.so"};

        if (info->dlpi_addr == 0 || info->dlpi_name == nullptr ||
            info->dlpi_name[0] == '\0') {
          return 0;
        } else if (ranges::any_of(excluded_paths,
                                  [&](auto b) { return fname == b; })) {
          return 0;
        }

        modules->emplace_back(std::string(info->dlpi_name), info->dlpi_addr);

        return 0;
      },
      &modules);

  marl::WaitGroup wg(modules.size());
  TracyLockable(std::mutex, vtable_mutex);
  for (auto& module : modules) {
    marl::schedule([=, &vtable_mutex, this] {
      ZoneScopedN("handle module");
      defer(wg.done());

      const std::string fname = basename(module.name.c_str());

      try {
        LoadedModule loaded_mod{
            module.name,
            static_cast<std::uintptr_t>(module.addr),
        };

        ElfModuleEhFrameParser eh_frame{&loaded_mod};
        ElfModuleVtableDumper dumped_rtti{&loaded_mod, &eh_frame};

        for (auto& vtable : dumped_rtti.get_vtables()) {
          vtable_mutex.lock();
          defer(vtable_mutex.unlock());

          module_vtables[std::pair{fname, vtable.first}] = vtable.second;
        };

      } catch (std::exception& e) {
        fmt::print(
            "while handling module {}:\n"
            "\t{}\n",
            fname, e.what());
        throw;
      };
    });
  }

  wg.wait();

  auto duration = std::chrono::high_resolution_clock::now() - start_time;
  fmt::print("Handling modules took {}\n",
             std::chrono::duration_cast<std::chrono::milliseconds>(duration));
}
