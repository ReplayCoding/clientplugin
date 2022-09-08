#pragma once

#include <gelf.h>
#include <cstdint>
#include <map>

class ElfModuleRttiDumper {
 public:
  ElfModuleRttiDumper(const std::string details);
  ~ElfModuleRttiDumper() { elf_end(elf); }

 private:
  Elf* elf;
  size_t string_header_index;

  GElf_Addr offline_baseaddr;
  size_t online_baseaddr;
  GElf_Addr calculate_offline_baseaddr();
  std::uintptr_t get_online_address_from_offline(GElf_Addr offline_addr);

  std::uintptr_t eh_frame_hdr_addr;

  void handle_relocations(Elf_Scn* scn, GElf_Shdr* shdr);

  std::map<std::uintptr_t, std::string> relocations{};
};

void LoadRtti();
