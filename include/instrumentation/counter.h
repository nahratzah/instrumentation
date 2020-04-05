#ifndef INSTRUMENTATION_COUNTER_H
#define INSTRUMENTATION_COUNTER_H

#include <instrumentation/fwd.h>
#include <instrumentation/detail/metric_group.h>
#include <array>
#include <atomic>
#include <string>
#include <memory>

namespace instrumentation::detail {


class counter_impl
: public std::enable_shared_from_this<counter_impl>
{
  public:
  void inc(double d = 1.0) noexcept;
  auto get() const noexcept -> double;
  void collect(const metric_name& name, const tags& tags, collector& c);

  private:
  std::atomic<double> v_{ 0.0 };
};


} /* namespace instrumentation::detail */

namespace instrumentation {


class counter {
  friend detail::counter_impl;
  template<typename... LabelTypes> friend class counter_vector;

  public:
  counter() noexcept = default;

  void operator++() const noexcept;
  void operator++(int) const noexcept;
  void operator+=(double d) const noexcept;

  explicit operator bool() const noexcept;
  auto operator!() const noexcept -> bool;
  auto operator*() const -> double;

  private:
  std::shared_ptr<detail::counter_impl> impl_;
};


template<typename... LabelTypes>
class counter_vector {
  private:
  using group_type = detail::metric_group<detail::counter_impl, LabelTypes...>;

  public:
  counter_vector() noexcept = default;
  counter_vector(metric_name name, std::array<std::string, sizeof...(LabelTypes)> labels, std::string description = "");
  counter_vector(engine& e, metric_name name, std::array<std::string, sizeof...(LabelTypes)> labels, std::string description = "");
  counter_vector(std::string_view name, std::array<std::string, sizeof...(LabelTypes)> labels, std::string description = "");
  counter_vector(engine& e, std::string_view name, std::array<std::string, sizeof...(LabelTypes)> labels, std::string description = "");

  auto labels(const LabelTypes&... values) const -> counter;

  explicit operator bool() const noexcept;
  auto operator!() const noexcept -> bool;

  private:
  std::shared_ptr<group_type> impl_;
};


} /* namespace instrumentation */

#include "counter-inl.h"

#endif /* INSTRUMENTATION_COUNTER_H */
