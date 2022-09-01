#pragma once
#include <fmt/core.h>
#include <exception>

// An abuse of throwing to print out an error message
class StringError : public std::exception {
 public:
  StringError(const std::string message) : message(message.c_str()){};

  template <typename... T>
  StringError(const std::string format, T... Args) {
    message = fmt::format(format, Args...).c_str();
  }

  const char* what() const noexcept override { return message; };

 private:
  const char* message;
};
