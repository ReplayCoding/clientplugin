#pragma once

#include "offsets/offsets.hpp"
#include "util/data_range_checker.hpp"

class ElfModuleEhFrameParser {
 public:
  ElfModuleEhFrameParser(LoadedModule* module);
  DataRangeChecker as_data_range_checker() {
    DataRangeChecker ranges{base_address};
    for (auto& function_range : function_ranges) {
      ranges.add_range(function_range);
    }

    return ranges;
  };

 private:
  struct CieInfo {
    uint8_t fde_pointer_encoding;
  };

  static CieInfo handle_cie(const std::uintptr_t cie_address);
  static std::vector<DataRange> handle_eh_frame(
      const std::uintptr_t start_address,
      const std::uintptr_t end_address);

  std::vector<DataRange> function_ranges;

  // Cached
  std::uintptr_t base_address{};
};
