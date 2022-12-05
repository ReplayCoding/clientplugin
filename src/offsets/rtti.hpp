#pragma once

#include <absl/container/flat_hash_map.h>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "offsets.hpp"

class RttiManager {
 public:
  RttiManager();

  uintptr_t get_function(std::string module,
                              std::string name,
                              uint16_t vftable,
                              uint16_t function);

 private:
  // module, name = vftable ptrs
  absl::flat_hash_map<std::pair<std::string, std::string>,
                      std::vector<uintptr_t>>
      module_vtables{};
};

extern std::unique_ptr<RttiManager> g_RTTI;
