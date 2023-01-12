#pragma once

#include <bit>
#include <bitset>
#include <cstdint>
#include <memory>
#include <span>
#include <tracy/Tracy.hpp>

using range_bs_t = std::bitset<134217728>;  // 128 MiB

struct DataRange {
  DataRange(uintptr_t begin, uintptr_t length) : begin(begin), length(length) {}

  inline std::span<uint8_t> data_at_mem() {
    return std::span(std::bit_cast<uint8_t*>(begin), length);
  }

  uintptr_t begin;
  uintptr_t length;
};

class DataRangeChecker {
 public:
  DataRangeChecker(uintptr_t base = 0) {
    ZoneScoped;
    this->base = base;
    this->range = std::make_unique<range_bs_t>();
  }

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
