#pragma once

#include <fmt/chrono.h>
#include <fmt/core.h>
#include <chrono>

struct TimedScope_ {
  TimedScope_(const std::string& name)
      : name(name), start_time(std::chrono::high_resolution_clock::now()){};
  ~TimedScope_() {
    auto duration = std::chrono::high_resolution_clock::now() - start_time;
    fmt::print("{} took {}\n", name,
               std::chrono::duration_cast<std::chrono::milliseconds>(duration));
  }

  const std::string name;
  std::chrono::time_point<std::chrono::system_clock> start_time;
};

#define TimedScope(n) TimedScope_ __timed_scope(n);
