#pragma once

#include <fmt/core.h>
#include <cstdint>
#include <span>

void hexdump(const uint8_t* ptr, size_t size) {
  auto span = std::span(ptr, size);

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
