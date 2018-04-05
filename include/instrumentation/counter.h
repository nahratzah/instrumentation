#ifndef INSTRUMENTATION_COUNTER_H
#define INSTRUMENTATION_COUNTER_H

///\file
///\ingroup instrumentation

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string_view>

namespace instrumentation {


class counter {
 public:
  constexpr counter(std::string_view name, group* parent) noexcept
  : name(name),
    parent_(parent)
  {}

  const std::string_view name;

  void enable() {
    std::call_once(registered_, &counter::do_register_, this);
  }

  auto operator++()
  noexcept
  -> void {
    enable();
    value_.fetch_add(1u, std::memory_order_relaxed);
  }

  auto operator++(int)
  noexcept
  -> void {
    ++*this;
  }

  auto operator*() const
  noexcept
  -> std::uintmax_t {
    return value_.load(std::memory_order_relaxed);
  }

 private:
  void do_register_() noexcept;

  group*const parent_;
  std::once_flag registered_;
  std::atomic<std::uintmax_t> value_{ 0u };
};


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_COUNTER_H */
