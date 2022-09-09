#pragma once
#include <string>

// TODO: Currently read only
class MemoryMappedFile {
 public:
  MemoryMappedFile(std::string fname);
  ~MemoryMappedFile();
  inline void* address() { return mapped_address; }
  inline size_t length() { return mapped_length; }

 private:
  size_t mapped_length;
  void* mapped_address;
};
