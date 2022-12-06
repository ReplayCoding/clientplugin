#pragma once

#include <absl/container/flat_hash_map.h>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "offsets/offsets.hpp"
#include "util/data_range_checker.hpp"

class RttiManager {
 public:
  RttiManager();

  uintptr_t get_function(std::string module,
                         std::string name,
                         uint16_t vftable,
                         uint16_t function) {
    auto vftable_ptr = module_vtables.at(module).at(name).at(vftable);
    return *reinterpret_cast<uintptr_t*>(vftable_ptr +
                                         (sizeof(void*) * function));
  };

 private:
  absl::flat_hash_map<std::string,
                      absl::flat_hash_map<std::string, std::vector<uintptr_t>>>
      module_vtables{};
};

extern std::unique_ptr<RttiManager> g_RTTI;
