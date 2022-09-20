#include <fmt/format.h>
#include <cstdint>
#include <utility>
#include <vector>

#include "util/data_range_checker.hpp"

void DataRangeChecker::add_range(std::uintptr_t start, std::uintptr_t length) {
  ranges.emplace_back(DataRange(start, length));
}

bool DataRangeChecker::is_position_in_range(std::uintptr_t position) {
  for (auto& range : ranges) {
    if (position >= range.start && position <= range.end)
      return true;
  }
  return false;
}

void DataRangeChecker::optimise_ranges() {
  constexpr auto LEEWAY =
      sizeof(void*);  // not enough to let a vtable slip through,
                      // should probably be elsewhere

  if (ranges.size() <= 1)
    return;

  fmt::print("BAH: data range is {} elems\n", ranges.size());

  // sort old ranges in place, don't copy
  std::sort(ranges.begin(), ranges.end());

  std::vector<DataRange> new_ranges{};

  DataRange current_optimised_range{ranges[0]};
  for (auto& range : ranges) {
    auto start_end_difference = current_optimised_range.end - range.start;
    if (start_end_difference <= LEEWAY) {
      current_optimised_range.end = range.end;
    } else {
      new_ranges.emplace_back(current_optimised_range);
      current_optimised_range = range;
    }
  }

  ranges = new_ranges;
  fmt::print("WOO: new data range is {} elems\n", new_ranges.size());
}
