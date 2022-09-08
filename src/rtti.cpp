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
  // This really really looks like a memory leak :(
  Elf_Data* section_data = elf_getdata(scn, nullptr);
  if (section_data == nullptr) {
    throw StringError("failed to get data for section {}: {}", shdr->sh_name);
  };

  auto reloc_struct_size = gelf_fsize(elf, ELF_T_REL, 1, EV_CURRENT);
  auto number_of_relocs = shdr->sh_size / reloc_struct_size;

  // really well designed api we have here
  Elf_Scn* symbol_section;
  if ((symbol_section = elf_getscn(elf, shdr->sh_link)) == nullptr)
    throw StringError("failed to get sh_link section");

  GElf_Shdr symbol_section_header;
  if (gelf_getshdr(symbol_section, &symbol_section_header) == nullptr)
    throw StringError("failed to get sh_link section header");

  Elf_Data* symbol_section_data = elf_getdata(symbol_section, nullptr);

  for (size_t relidx = 0; relidx < number_of_relocs; ++relidx) {
    GElf_Rel rel;
    if (gelf_getrel(section_data, relidx, &rel) == nullptr) {
      throw StringError("couldn't get relocation for section {}: {}",
                        elf_strptr(elf, string_header_index, shdr->sh_name),
                        relidx);
    }

    if (GELF_R_TYPE(rel.r_info) != 0 /* NONE rel type */ &&
        GELF_R_SYM(rel.r_info) != STN_UNDEF /* no symbol */) {
      GElf_Sym symbol;
      if (gelf_getsymshndx(symbol_section_data, nullptr, GELF_R_SYM(rel.r_info),
                           &symbol, nullptr) == nullptr)
        throw StringError("Failed to get symbol index for reloc: {}", relidx);

      auto symbol_name =
          elf_strptr(elf, symbol_section_header.sh_link, symbol.st_name);
      if (symbol_name == nullptr)
        throw StringError("Failed to get symbol name");

      fmt::print("got reloc offset: {:08X} (type: {}, sym: {}, sym name: {})\n",
                 rel.r_offset, GELF_R_TYPE(rel.r_info), GELF_R_SYM(rel.r_info),
                 symbol_name);
    }
  }
}

ElfRttiDumper::ElfRttiDumper(const std::string path) {
  // Ugh, i don't want to use a unique_ptr. FIXME
  std::unique_ptr<MemoryMappedFile> mapped_file;

  try {
    mapped_file = std::make_unique<MemoryMappedFile>(path);
  } catch (std::exception& e) {
    throw StringError("WARNING: failed to load module {} ({})", path, e.what());
  }

  elf = elf_memory(static_cast<char*>(mapped_file->getAddress()),
                   mapped_file->getLength());
  // If we failed to load the file, just continue on
  if (elf == nullptr) {
    throw StringError("note: failed to load module: {}", path);
  };

  if (elf_getshdrstrndx(elf, &string_header_index) < 0) {
    throw StringError("failed to get sh string section");
  };

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
