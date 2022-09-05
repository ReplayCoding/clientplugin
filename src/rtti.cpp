#include <elf.h>
#include <fcntl.h>
#include <fmt/core.h>
#include <frida-gum.h>
#include <gelf.h>
#include <libelf.h>
#include <cassert>
#include <cstdint>
#include <string_view>

#include "rtti.hpp"
#include "util/error.hpp"
#include "util/mmap.hpp"

GElf_Addr ElfRttiDumper::calculate_offline_baseaddr() {
  GElf_Ehdr ehdr;
  gelf_getehdr(elf, &ehdr);

  auto phdr_count = ehdr.e_phnum;

  for (auto idx = 0; idx != phdr_count; idx++) {
    GElf_Phdr phdr;
    gelf_getphdr(elf, idx, &phdr);

    if (phdr.p_type == PT_LOAD && phdr.p_offset == 0) {
      return phdr.p_vaddr;
    }
  }

  // We failed to find a base address for this binary
  throw StringError("Failed to find base address");
}

std::uintptr_t ElfRttiDumper::get_online_address_from_offline(
    GElf_Addr offline_addr) {
  return online_baseaddr + (offline_addr - offline_baseaddr);
}

void ElfRttiDumper::handle_relocations(Elf_Scn* scn, GElf_Shdr* shdr) {
  auto reloc_struct_size = gelf_fsize(elf, ELF_T_REL, 1, EV_CURRENT);
  auto number_of_relocs = shdr->sh_size / reloc_struct_size;
  (void)number_of_relocs;
}

ElfRttiDumper::ElfRttiDumper(const std::string path) {
  // Ugh, i don't want to use a unique_ptr. FIXME
  std::unique_ptr<MemoryMappedFile> mapped_file;
  try {
    mapped_file = std::make_unique<MemoryMappedFile>(path);
  } catch (std::exception& e) {
    throw StringError("WARNING: failed to load module {} ({})\n", path,
                      e.what());
  }

  elf = elf_memory(static_cast<char*>(mapped_file->getAddress()),
                   mapped_file->getLength());
  // If we failed to load the file, just continue on
  if (elf == nullptr) {
    throw StringError("note: failed to load module: {}\n",
                      elf_errmsg(elf_errno()));
  };

  elf_getshdrstrndx(elf, &string_header_index);

  // Find online & offline load addresses
  online_baseaddr = gum_module_find_base_address(path.c_str());
  try {
    offline_baseaddr = calculate_offline_baseaddr();
  } catch (std::exception& e) {
    elf_end(elf);
    throw StringError("WARNING: {}", e.what());
  }
  if (online_baseaddr == 0) {
    // bail out, this is bad
    elf_end(elf);
    throw StringError("WARNING: online_baseaddress is nullptr");
  };

  fmt::print("Got baddr for {}: offline is {:08X}, online is {:08X}\n", path,
             offline_baseaddr, online_baseaddr);

  // if the section is null, we read the first section
  Elf_Scn* cur_scn = nullptr;
  while ((cur_scn = elf_nextscn(elf, cur_scn)) != nullptr) {
    GElf_Shdr hdr{};
    if (gelf_getshdr(cur_scn, &hdr) == nullptr)
      throw StringError("Error getting section {}", elf_ndxscn(cur_scn));
    std::string_view section_name =
        elf_strptr(elf, string_header_index, hdr.sh_name);

    if (hdr.sh_type == SHT_REL) {
      fmt::print("Found SHT_REL: {}\n", section_name);
      handle_relocations(cur_scn, &hdr);
    }

    if (section_name == ".eh_frame_hdr") {
      eh_frame_hdr_addr = get_online_address_from_offline(hdr.sh_addr);
      fmt::print(
          "Got desired sections for {}:\n"
          "\t .eh_frame_hdr: {:08X}\n",
          path, eh_frame_hdr_addr);
    }
  }
}

void LoadRtti() {
  elf_version(EV_CURRENT);
  gum_process_enumerate_modules(
      [](const GumModuleDetails* details, void* user_data) {
        // ignore vdso
        constexpr std::string_view vdso_path = "linux-vdso.so.1";
        if (vdso_path == details->name)
          return 1;

        try {
          ElfRttiDumper dumped_rtti{details->path};
        } catch (std::exception& e) {
          std::string_view{e.what()};
          fmt::print(
              "while handling module {}:\n"
              "\t{}\n",
              details->name, e.what());
        };
        // this is stupid
        return 1;
      },
      nullptr);
}
