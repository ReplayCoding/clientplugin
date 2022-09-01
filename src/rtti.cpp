#include <elf.h>
#include <fcntl.h>
#include <fmt/core.h>
#include <frida-gum.h>
#include <gelf.h>
#include <libelf.h>
#include <cassert>

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

  // Yes, we are indeed pulling in libelf just to get the section address
  auto* elf = elf_memory(static_cast<char*>(mapped_file->getAddress()),
                         mapped_file->getLength());
  // If we failed to load the file, just continue on
  if (elf == nullptr) {
    fmt::print("note: failed to load module {}: {}\n", details->name,
               elf_errmsg(elf_errno()));
    return true;
  };

  auto offline_baseaddr = get_offline_baseaddr(elf);
  auto online_baseaddr = gum_module_find_base_address(details->name);
  if (online_baseaddr == 0) {
    // bail out, this is bad
    elf_end(elf);
    return true;
  };

  fmt::print("Got baddr for {}: offline is {:08X}, online is {:08X}\n",
             details->name, offline_baseaddr, online_baseaddr);

  // if the section is null, we read the first section
  Elf_Scn* cur_scn = nullptr;
  while ((cur_scn = elf_nextscn(elf, cur_scn)) != nullptr) {
    GElf_Shdr hdr;
    gelf_getshdr(cur_scn, &hdr);

    auto online_section_addr =
        online_baseaddr + (hdr.sh_addr - offline_baseaddr);
    (void)online_section_addr;
  };

  elf_end(elf);

  return true;
}

void fuckywucky() {
  elf_version(EV_CURRENT);
  gum_process_enumerate_modules(found_module_handler, nullptr);
}
