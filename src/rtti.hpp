#pragma once

#include <gelf.h>
#include <cstdint>
#include <map>
#include <memory>

#include "util/data_range_checker.hpp"
#include "util/mmap.hpp"

class ElfModuleRttiDumper {
 public:
  ElfModuleRttiDumper(const std::string details);
  ~ElfModuleRttiDumper() { elf_end(elf); }

 private:
  struct cie_info_t {
    uint8_t fde_pointer_encoding;
  };

  GElf_Addr calculate_offline_baseaddr();
  std::uintptr_t get_online_address_from_offline(GElf_Addr offline_addr);

  void handle_relocations(Elf_Scn* scn, GElf_Shdr* shdr);

  cie_info_t handle_cie(const std::uintptr_t cie_address);

  DataRangeChecker handle_eh_frame(const std::uintptr_t start_address,
                                   const std::uintptr_t end_address);

  // keep this in memory until we free elf
  std::unique_ptr<MemoryMappedFile> mapped_file{};
  Elf* elf{};

  size_t string_header_index{};

  GElf_Addr offline_baseaddr{};
  size_t online_baseaddr{};

  std::map<std::uintptr_t, std::string> relocations{};
  DataRangeChecker function_ranges;
};

void LoadRtti();
