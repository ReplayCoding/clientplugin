#pragma once

#include <absl/container/flat_hash_map.h>
#include <cstddef>
#include <cstdint>
#include <elfio/elfio.hpp>
#include <memory>
#include <vector>

#include "offsets.hpp"
#include "offsets/eh_frame.hpp"
#include "util/data_range_checker.hpp"
#include "util/generator.hpp"
#include "util/mmap.hpp"

Generator<DataRange> section_ranges(LoadedModule& loaded_mod);

using RelocMap = absl::flat_hash_map<std::uintptr_t, std::string>;
Generator<std::pair<std::uintptr_t, std::string>> get_relocations(
    LoadedModule& loaded_mod,
    ELFIO::section* section);

size_t get_typeinfo_size(RelocMap& relocations, std::uintptr_t size);
Generator<std::uintptr_t> locate_vftables(LoadedModule& loaded_mod,
                                          RelocMap& relocations,
                                          DataRangeChecker& function_ranges);

using Vtable = std::pair<std::string, std::vector<std::uintptr_t>>;
Generator<Vtable> get_vtables_from_module(LoadedModule& loaded_mod,
                                          ElfModuleEhFrameParser& eh_frame);

class RttiManager {
 public:
  RttiManager();

  std::uintptr_t get_function(std::string module,
                              std::string name,
                              uint16_t vftable,
                              uint16_t function);

 private:
  // module, name = vftable ptrs
  absl::flat_hash_map<std::pair<std::string, std::string>,
                      std::vector<std::uintptr_t>>
      module_vtables{};
};

extern std::unique_ptr<RttiManager> g_RTTI;
