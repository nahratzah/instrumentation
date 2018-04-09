#ifndef INSTRUMENTATION_COUNTER_H
#define INSTRUMENTATION_COUNTER_H

///\file
///\ingroup instrumentation

#include <cstdint>
#include <instrumentation/hierarchy.h>
#include <instrumentation/instrumentation_export_.h>

namespace instrumentation {


class instrumentation_export_ counter final
: public hierarchy
{
 public:
  using hierarchy::hierarchy;

  ~counter() noexcept override;

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

  auto visit(visitor& v) const -> void override;

 private:
  std::atomic<std::uintmax_t> value_{ 0u };
};


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_COUNTER_H */
