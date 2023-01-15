#include <elfio/elfio.hpp>
#include <string>
#include <tracy/Tracy.hpp>

#include "util/data_range_checker.hpp"
#include "util/error.hpp"
#include "util/loaded_module.hpp"

LoadedModule::LoadedModule(const std::string& path,
                           const DataRange& mod_range) {
  ZoneScoped;

  elf.load(path);

  online_baseaddr = mod_range.begin;
  online_length = mod_range.length;
  for (auto& segment : elf.segments) {
    if (segment->get_type() == ELFIO::PT_LOAD) {
      offline_baseaddr =
          std::min(offline_baseaddr,
                   static_cast<uintptr_t>(segment->get_virtual_address()));
    }
  }

  if (online_baseaddr == 0) {
    // bail out, this is bad
    throw StringError("WARNING: loaded_mod.base_address is nullptr");
  };
}
