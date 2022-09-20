#pragma once

#include <bitset>
#include <cstdint>
#include <utility>

constexpr auto data_range_size = 1073741824;

class DataRangeChecker {
 public:
  DataRangeChecker(std::uintptr_t base_ = 0) : base(base_) {}
  void add_range(std::uintptr_t start, std::uintptr_t length);
  bool is_position_in_range(std::uintptr_t position);

 private:
  std::uintptr_t base{};
  std::bitset<data_range_size> range{};
};
