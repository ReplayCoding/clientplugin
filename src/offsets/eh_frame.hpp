#pragma once

#include "offsets/offsets.hpp"
#include "util/data_range_checker.hpp"
#include "util/generator.hpp"

struct CieInfo {
  uint8_t fde_pointer_encoding;
};

CieInfo handle_cie(const uintptr_t cie_address);
Generator<DataRange> handle_eh_frame(const uintptr_t start_address,
                                     const uintptr_t end_address);

Generator<DataRange> get_eh_frame_ranges(LoadedModule& module);
