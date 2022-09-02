#include <elf.h>
#include <fcntl.h>
#include <fmt/core.h>
#include <frida-gum.h>
#include <gelf.h>
#include <libelf.h>
#include <cassert>
#include <cstdint>
#include <string_view>

#include "util/error.hpp"
#include "util/mmap.hpp"

GElf_Addr get_offline_baseaddr(Elf* elf) {
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
  throw StringError("Failed to find base address (cant get name here)");
}

// gboolean is an int, WTF?
// This function should ALWAYS return true, no matter what.
int found_module_handler(const GumModuleDetails* details, void* user_data) {
  // ignore vdso
  const std::string vdso_path = "linux-vdso.so.1";
  if (vdso_path == details->name)
    return true;

  // Ugh. FIXME
  std::unique_ptr<MemoryMappedFile> mapped_file;
  try {
    mapped_file = std::make_unique<MemoryMappedFile>(details->path);
  } catch (std::exception& e) {
    fmt::print("WARNING: failed to load module {}\n", details->path);
    return true;
  }

  auto* elf = elf_memory(static_cast<char*>(mapped_file->getAddress()),
                         mapped_file->getLength());
  // If we failed to load the file, just continue on
  if (elf == nullptr) {
    fmt::print("note: failed to load module {}: {}\n", details->name,
               elf_errmsg(elf_errno()));
    return true;
  };

  // Find online & offline load addresses
  auto online_baseaddr = gum_module_find_base_address(details->name);
  GElf_Addr offline_baseaddr;
  try {
    offline_baseaddr = get_offline_baseaddr(elf);
  } catch (std::exception& e) {
    fmt::print("WARNING: {}", e.what());
    return true;
  }
  if (online_baseaddr == 0) {
    // bail out, this is bad
    elf_end(elf);
    return true;
  };

  fmt::print("Got baddr for {}: offline is {:08X}, online is {:08X}\n",
             details->name, offline_baseaddr, online_baseaddr);

  size_t string_header_index{};
  elf_getshdrstrndx(elf, &string_header_index);

  // if the section is null, we read the first section
  // This is incremented every time we find a section we want
  // TODO: Find a better way to do this
  constexpr auto desired_sections = 2;  // .eh_frame and .eh_frame_hdr
  unsigned int grabbed_sections{};
  std::uintptr_t eh_frame_addr;
  std::uintptr_t eh_frame_hdr_addr;

  Elf_Scn* cur_scn = nullptr;
  while ((cur_scn = elf_nextscn(elf, cur_scn)) != nullptr) {
    GElf_Shdr hdr;
    gelf_getshdr(cur_scn, &hdr);

    auto online_section_addr =
        online_baseaddr + (hdr.sh_addr - offline_baseaddr);
    std::string_view section_name =
        elf_strptr(elf, string_header_index, hdr.sh_name);
    if (section_name == ".eh_frame") {
      eh_frame_addr = online_section_addr;
      grabbed_sections++;
    } else if (section_name == ".eh_frame_hdr") {
      eh_frame_hdr_addr = online_section_addr;
      grabbed_sections++;
    }
  }

  if (desired_sections == grabbed_sections) {
    fmt::print(
        "Got desired sections for {}:\n"
        "\t .eh_frame:     {:08X}\n"
        "\t .eh_frame_hdr: {:08X}\n",
        details->name, eh_frame_addr, eh_frame_hdr_addr);
  }

  elf_end(elf);

  return true;
}

void fuckywucky() {
  elf_version(EV_CURRENT);
  gum_process_enumerate_modules(found_module_handler, nullptr);
}
