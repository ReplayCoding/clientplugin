#include <fcntl.h>
#include <fmt/core.h>
#include <frida-gum.h>
#include <unistd.h>

#include "util.hpp"

// gboolean is an int, WTF?
int found_module_handler(const GumModuleDetails* details, void* user_data) {
  fmt::print("WE FOUND A MODULE: {}.\n", details->name);
  // gum_module_enumerate_ranges(details->name, GUM_PAGE_READ, nullptr, nullptr);

  return true;
}

void fuckywucky() {
  gum_process_enumerate_modules(found_module_handler, nullptr);
}