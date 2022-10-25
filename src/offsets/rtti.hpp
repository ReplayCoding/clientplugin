#pragma once

#include <gelf.h>
#include <libelf.h>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

#include "util/data_range_checker.hpp"
#include "util/mmap.hpp"

class ElfModuleVtableDumper {
 public:
  ElfModuleVtableDumper(const std::string path, size_t baddr, size_t size);
  ~ElfModuleVtableDumper() { elf_end(elf); }

  using vtables_t =
      std::unordered_map<std::string, std::vector<std::uintptr_t>>;
  vtables_t get_vtables();

 private:
  struct cie_info_t {
    uint8_t fde_pointer_encoding;
  };

  void generate_data_from_sections();

  GElf_Addr calculate_offline_baseaddr();
  std::uintptr_t get_online_address_from_offline(GElf_Addr offline_addr);
  std::uintptr_t get_offline_address_from_online(std::uintptr_t online_addr);

  void get_relocations(Elf_Scn* scn, GElf_Shdr* shdr);

  cie_info_t handle_cie(const std::uintptr_t cie_address);

  DataRangeChecker handle_eh_frame(const std::uintptr_t start_address,
                                   const std::uintptr_t end_address);

  size_t get_typeinfo_size(std::uintptr_t size);

  std::vector<std::uintptr_t> locate_vftables();

  // keep this in memory until we free elf
  std::unique_ptr<MemoryMappedFile> mapped_file{};
  Elf* elf{};

  size_t string_header_index{};

  GElf_Addr offline_baseaddr{};

  size_t online_baseaddr{};
  size_t online_size{};

  std::unordered_map<std::uintptr_t, std::string> relocations{};
  DataRangeChecker function_ranges;
  DataRangeChecker section_ranges;
};

class RttiManager {
 public:
  RttiManager();

  // This is stupid.
  inline void submit_vtables(std::string mname,
                             ElfModuleVtableDumper::vtables_t vtables) {
    module_vtables[mname] = vtables;
  };

  std::uintptr_t get_function(std::string module,
                              std::string name,
                              uint16_t vftable,
                              uint16_t function);

 private:
  std::unordered_map<std::string, ElfModuleVtableDumper::vtables_t>
      module_vtables{};
};

extern std::unique_ptr<RttiManager> g_RTTI;
