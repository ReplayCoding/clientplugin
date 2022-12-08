#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "offsets/offsets.hpp"
#include "util/data_range_checker.hpp"
#include "util/generator.hpp"

using Vtable = std::pair<std::string, std::vector<uintptr_t>>;
Generator<Vtable> get_vtables_from_module(LoadedModule& loaded_mod);
