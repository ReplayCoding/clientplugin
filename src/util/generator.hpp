#pragma once
#include <coroutine>
#include <cstddef>
#include <exception>
#include <iterator>
#include <optional>
#include <utility>
#include <variant>

template <typename T>
struct Generator {
  struct promise_type {
    auto get_return_object() noexcept { return Generator{*this}; };
    std::suspend_always initial_suspend() const noexcept { return {}; };
    std::suspend_always final_suspend() const noexcept { return {}; };

    void unhandled_exception() { result = std::current_exception(); };

    // template <std::convertible_to<T> From>
    // std::suspend_always yield_value(const From& value) {
    //   result = value;
    //   return {};
    // }

    template <std::convertible_to<T> From>
    std::suspend_always yield_value(From&& value) {
      result = std::addressof(value);
      return {};
    }

    void return_void() const noexcept {}

    void throw_if_exception() {
      if (std::holds_alternative<std::exception_ptr>(result))
        std::rethrow_exception(std::get<std::exception_ptr>(result));
    }

    bool has_value() { return !std::holds_alternative<std::monostate>(result); }
    T& get_value() {
      return std::holds_alternative<T>(result) ? std::get<T>(result)
                                               : *std::get<T*>(result);
    }

   private:
    std::variant<std::monostate, T, T*, std::exception_ptr> result;
  };

  struct Iterator {
    using iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using reference = T&;
    using pointer = T*;

    Iterator() noexcept = default;
    explicit Iterator(const std::coroutine_handle<promise_type>& coro) noexcept
        : coro{&coro} {}

    friend bool operator==(const Iterator&, const Iterator&) noexcept = default;
    Iterator& operator++() {
      assert(coro != nullptr);
      assert(!coro->done());

      coro->resume();
      if (coro->done()) {
        auto handle = std::exchange(coro, nullptr);
        handle->promise().throw_if_exception();
      }

      return *this;
    }

    T& operator*() const noexcept {
      assert(coro != nullptr);
      assert(!coro->done());
      return coro->promise().get_value();
    }

   private:
    const std::coroutine_handle<promise_type>* coro;
  };

  Generator(Generator&& other) noexcept
      : coro{std::exchange(other.coro, nullptr)} {}

  Generator& operator=(Generator&& other) noexcept {
    if (coro)
      coro.destroy();
    coro = std::exchange(other.coro, nullptr);
  };

  ~Generator() {
    if (coro)
      coro.destroy();
  };

  // auto& operator()() const {
  //   coro.resume();
  //   return coro.promise().get_value();
  // };

  Iterator begin() const {
    if (coro.done())
      return end();

    auto i = Iterator(coro);
    if (!coro.promise().has_value())
      ++i;

    return i;
  };
  Iterator end() const noexcept { return Iterator{}; };

 private:
  explicit Generator(promise_type& promise) noexcept
      : coro{std::coroutine_handle<promise_type>::from_promise(promise)} {};

  std::coroutine_handle<promise_type> coro;
};
