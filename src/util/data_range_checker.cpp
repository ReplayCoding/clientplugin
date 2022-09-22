#include <bitset>
#include <cstdint>
#include <utility>

#include "util/data_range_checker.hpp"

void DataRangeChecker::add_range(std::uintptr_t start, std::uintptr_t length) {
  for (auto i = start; i < start + length; i++) {
    range->set(i - base);
  }
}

bool DataRangeChecker::is_position_in_range(std::uintptr_t position) {
  return range->test(position - base);
}
