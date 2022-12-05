#pragma once

#include <bitset>
#include <cstdint>
#include <memory>
#include <utility>

using range_bs_t =
    std::bitset<1073741824>;  // Can represent 1 GiB of data (more than enough
                              // to hold even LLVM's memory space)

struct DataRange {
  DataRange(uintptr_t begin, uintptr_t length)
      : begin(begin), length(length) {}

  uintptr_t begin;
  uintptr_t length;
};

class DataRangeChecker {
 public:
  DataRangeChecker(uintptr_t base = 0)
      : base(base), range(std::make_unique<range_bs_t>()) {}

  inline void add_range(const DataRange& r) {
    for (auto i = r.begin; i < r.begin + r.length; i++) {
      range->set(i - base);
    }
  }

  inline bool is_position_in_range(uintptr_t position) {
    return range->test(position - base);
  }

 private:
  uintptr_t base{};
  std::unique_ptr<range_bs_t> range{};
};
