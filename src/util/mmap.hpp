#pragma once

// TODO: Currently read only
class MemoryMappedFile {
 public:
  MemoryMappedFile(std::string fname);
  ~MemoryMappedFile();
  inline void* getAddress() { return mapped_address; }
  inline size_t getLength() { return mapped_length; }

 private:
  size_t mapped_length;
  void* mapped_address;
};
