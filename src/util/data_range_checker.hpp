#pragma once

#include <bitset>
#include <cstdint>
#include <memory>
#include <utility>

using range_bs_t =
    std::bitset<1073741824>;  // Can represent 1 GiB of data (more than enough
                              // to hold even LLVM's memory space)

class DataRangeChecker {
 public:
  DataRangeChecker(std::uintptr_t base_ = 0)
      : base(base_), range(std::make_unique<range_bs_t>()) {}

  inline void add_range(std::uintptr_t start, std::uintptr_t length) {
    for (auto i = start; i < start + length; i++) {
      range->set(i - base);
    }
  }

  inline bool is_position_in_range(std::uintptr_t position) {
    return range->test(position - base);
  }

 private:
  std::uintptr_t base{};
  std::unique_ptr<range_bs_t> range{};
};
