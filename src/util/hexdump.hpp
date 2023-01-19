#pragma once

#include <fmt/core.h>
#include <span>

void hexdump(const std::vector<uint8_t>& span) {
  for (auto row : span | ranges::views::chunk(16)) {
    for (auto b : row)
      fmt::print("{:02x} ", b);
    fmt::print(" | ");
    for (auto b : row) {
      auto c = static_cast<char>(b);
      if (c != '\n' && isprint(b))
        fmt::print("{:c}", b);
      else
        fmt::print(".");
    }
    fmt::print("\n");
  }
  fmt::print("\n---------------------\n");
}
