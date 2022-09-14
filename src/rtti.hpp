#pragma once

#include <gelf.h>
#include <cstdint>
#include <map>
#include <memory>

#include "util/mmap.hpp"

class ElfModuleRttiDumper {
 public:
  ElfModuleRttiDumper(const std::string details);
  ~ElfModuleRttiDumper() { elf_end(elf); }
  inline auto relocations() const { return _relocations; }

 private:
  // keep in memory until we free elf
  std::unique_ptr<MemoryMappedFile> mapped_file{};

  Elf* elf{};
  size_t string_header_index{};

  GElf_Addr offline_baseaddr{};
  size_t online_baseaddr{};

  std::map<std::uintptr_t, std::string> _relocations{};

  GElf_Addr calculate_offline_baseaddr();
  std::uintptr_t get_online_address_from_offline(GElf_Addr offline_addr);

  void handle_relocations(Elf_Scn* scn, GElf_Shdr* shdr);
  void handle_eh_frame(const std::uintptr_t start_address,
                       const std::uintptr_t end_address);
};

void LoadRtti();
