#pragma once

#include <absl/container/inlined_vector.h>
#include <cstdint>
#include <string_view>
#include <utility>
#include <vector>

#include "util/data_range_checker.hpp"
#include "util/generator.hpp"
#include "util/loaded_module.hpp"

// Max vftables i've observed in TF is 10
using Vftables = absl::InlinedVector<uintptr_t, 12>;
using Vtable = std::pair<std::string_view, Vftables>;

Generator<Vtable> get_vtables_from_module(LoadedModule& loaded_mod,
                                          std::span<DataRange> function_ranges);
