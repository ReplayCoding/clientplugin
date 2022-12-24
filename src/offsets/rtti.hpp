#pragma once

#include <cstdint>
#include <string_view>
#include <utility>
#include <vector>

#include "offsets/offsets.hpp"
#include "util/data_range_checker.hpp"
#include "util/generator.hpp"

using Vtable = std::pair<std::string_view, std::vector<uintptr_t>>;
Generator<Vtable> get_vtables_from_module(LoadedModule& loaded_mod,
                                          std::span<DataRange> function_ranges);
