#pragma once

#include "offsets/offsets.hpp"
#include "util/data_range_checker.hpp"

struct FunctionRange {
  FunctionRange(std::uintptr_t begin, std::uintptr_t size)
      : begin(begin), size(size) {}

  std::uintptr_t begin;
  std::uintptr_t size;
};

class ElfModuleEhFrameParser {
 public:
  ElfModuleEhFrameParser(LoadedModule* module);
  DataRangeChecker as_data_range() {
    DataRangeChecker ranges{base_address};
    for (auto& function_range : function_ranges) {
      ranges.add_range(function_range.begin, function_range.size);
    }

    return ranges;
  };

 private:
  struct CieInfo {
    uint8_t fde_pointer_encoding;
  };

  static CieInfo handle_cie(const std::uintptr_t cie_address);
  static std::vector<FunctionRange> handle_eh_frame(
      const std::uintptr_t start_address,
      const std::uintptr_t end_address);

  std::vector<FunctionRange> function_ranges;

  // Cached
  std::uintptr_t base_address{};
};
