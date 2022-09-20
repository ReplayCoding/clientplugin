#pragma once

#include <cstdint>
#include <utility>
#include <vector>

class DataRange {
 public:
  DataRange(std::uintptr_t start_, std::uintptr_t length)
      : start(start_), end(start_ + length) {}

  bool operator<(DataRange& other) { return start < other.start; }

  std::uintptr_t start;
  std::uintptr_t end;
};

class DataRangeChecker {
 public:
  DataRangeChecker() = default;
  void add_range(std::uintptr_t start, std::uintptr_t length);
  bool is_position_in_range(std::uintptr_t position);
  void optimise_ranges();

 private:
  std::vector<DataRange> ranges{};
};
