#pragma once
#include <cstdint>
#include <elfio/elfio.hpp>
#include <string>

#include "util/data_range_checker.hpp"

struct LoadedModule {
  LoadedModule(const std::string& path, const DataRange& mod_range);

  inline uintptr_t get_online_address_from_offline(uintptr_t offline_addr) {
    return online_baseaddr + (offline_addr - offline_baseaddr);
  }

  inline uintptr_t get_offline_address_from_online(uintptr_t online_addr) {
    return offline_baseaddr + (online_addr - online_baseaddr);
  }

  ELFIO::elfio elf{};

  uintptr_t online_baseaddr{};
  uintptr_t online_length{};

 private:
  uintptr_t offline_baseaddr{UINTPTR_MAX};
};
