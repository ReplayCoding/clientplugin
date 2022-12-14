#pragma once
#include <coroutine>
#include <exception>
#include <range/v3/detail/range_access.hpp>
#include <range/v3/iterator/default_sentinel.hpp>
#include <range/v3/view/facade.hpp>
#include <variant>

template <typename T>
struct Generator : public ranges::view_facade<Generator<T>> {
  friend ranges::range_access;
  struct promise_type {
    auto get_return_object() noexcept { return Generator{*this}; };
    std::suspend_always initial_suspend() const noexcept { return {}; };
    std::suspend_always final_suspend() const noexcept { return {}; };

    void unhandled_exception() { result = std::current_exception(); };

    template <std::convertible_to<T> From>
    std::suspend_always yield_value(const From& value) {
      result = std::addressof(value);
      return {};
    }

    template <std::convertible_to<T> From>
    std::suspend_always yield_value(From&& value) {
      result = std::move(value);
      return {};
    }

    void return_void() noexcept { result = std::monostate(); }

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

  struct Cursor {
    explicit Cursor(const std::coroutine_handle<promise_type>& coro) noexcept
        : coro{&coro} {}

    bool equal(ranges::default_sentinel_t) const {
      if (coro->done()) {
        coro->promise().throw_if_exception();
        return true;
      }

      return false;
    };

    void next() {
      assert(coro != nullptr);
      assert(!coro->done());

      coro->resume();
    }

    T& read() const {
      assert(coro != nullptr);

      return coro->promise().get_value();
    }

   private:
    const std::coroutine_handle<promise_type>* coro;
  };

  ~Generator() {
    if (coro)
      coro.destroy();
  };

  Cursor begin_cursor() const {
    auto cur = Cursor(coro);

    // Get initial value.
    cur.next();

    return cur;
  };

 private:
  explicit Generator() noexcept = default;
  explicit Generator(promise_type& promise) noexcept
      : coro{std::coroutine_handle<promise_type>::from_promise(promise)} {};

  std::coroutine_handle<promise_type> coro;
};
