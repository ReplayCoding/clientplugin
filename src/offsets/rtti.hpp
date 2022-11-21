#pragma once

#include <cstddef>
#include <cstdint>
#include <elfio/elfio.hpp>
#include <memory>
#include <vector>
#include <absl/container/flat_hash_map.h>

#include "offsets.hpp"
#include "offsets/eh_frame.hpp"
#include "util/data_range_checker.hpp"
#include "util/mmap.hpp"

class ElfModuleVtableDumper {
 public:
  ElfModuleVtableDumper(LoadedModule* module, ElfModuleEhFrameParser* eh_frame);

  using Vtables = absl::flat_hash_map<std::string, std::vector<std::uintptr_t>>;
  Vtables get_vtables();

 private:
  void generate_data_from_sections();
  void get_relocations(ELFIO::section* section);
  size_t get_typeinfo_size(std::uintptr_t size);
  std::vector<std::uintptr_t> locate_vftables();

  LoadedModule* loaded_mod;

  absl::flat_hash_map<std::uintptr_t, std::string> relocations{};
  DataRangeChecker function_ranges;
  DataRangeChecker section_ranges;
};

class RttiManager {
 public:
  RttiManager();

  // This is stupid.
  inline void submit_vtables(std::string mname,
                             ElfModuleVtableDumper::Vtables vtables) {
    module_vtables[mname] = vtables;
  };

  std::uintptr_t get_function(std::string module,
                              std::string name,
                              uint16_t vftable,
                              uint16_t function);

 private:
  absl::flat_hash_map<std::string, ElfModuleVtableDumper::Vtables>
      module_vtables{};
};

extern std::unique_ptr<RttiManager> g_RTTI;
