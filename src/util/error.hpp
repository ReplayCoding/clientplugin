#pragma once
#include <fmt/core.h>
#include <exception>

class StringError : public std::exception {
 public:
  StringError(const std::string& message) : message(message){};

  template <typename... T>
  StringError(const std::string& format, T... Args) {
    message = fmt::format(fmt::runtime(format), Args...);
  }

  const char* what() const noexcept override { return message.c_str(); };

 private:
  std::string message;
};
