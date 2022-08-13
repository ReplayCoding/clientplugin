#pragma once
#include <exception>

class StringError : public std::exception {
 public:
  StringError(const char* message) : message(message){};
  const char* what() const noexcept override { return message; };

 private:
  const char* message;
};
