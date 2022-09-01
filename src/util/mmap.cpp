#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "./mmap.hpp"
#include "util/error.hpp"

MemoryMappedFile::MemoryMappedFile(std::string fname) {
  auto fd = open(fname.c_str(), O_RDONLY);
  if (fd < 0)
    throw StringError("Failed to open file: {}", fname);

  mapped_length = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);

  mapped_address = mmap(nullptr, mapped_length, PROT_READ, MAP_PRIVATE, fd, 0);
  if (mapped_address == MAP_FAILED)
    throw StringError("Failed to mmap file: {}", fname);

  // We can close the file without invalidating the mapping
  close(fd);
}

MemoryMappedFile::~MemoryMappedFile() {
  munmap(mapped_address, mapped_length);
}
