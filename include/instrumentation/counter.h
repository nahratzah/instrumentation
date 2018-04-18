#ifndef INSTRUMENTATION_COUNTER_H
#define INSTRUMENTATION_COUNTER_H

///\file
///\ingroup instrumentation

#include <cstddef>
#include <cstdint>
#include <instrumentation/basic_metric.h>
#include <instrumentation/tags.h>
#include <instrumentation/instrumentation_export_.h>

namespace instrumentation {


class instrumentation_export_ counter final
: public basic_metric
{
 public:
  counter(std::string_view local_name, group& parent, const tag_map& t = tag_map()) noexcept
  : basic_metric(local_name, parent, t)
  {
    this->enable();
  }

  ~counter() noexcept override;

  auto operator++()
  noexcept
  -> void {
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
