#ifndef INSTRUMENTATION_GAUGE_H
#define INSTRUMENTATION_GAUGE_H

#include <instrumentation/fwd.h>
#include <instrumentation/detail/metric_group.h>
#include <array>
#include <atomic>
#include <string>
#include <memory>

namespace instrumentation::detail {


class gauge_impl
: public std::enable_shared_from_this<gauge_impl>
{
  public:
  void inc(double d = 1.0) noexcept;
  void dec(double d = 1.0) noexcept;
  void set(double d) noexcept;
  auto get() const noexcept -> double;
  void collect(const metric_name& name, const tags& tags, collector& c);

  private:
  std::atomic<double> v_{ 0.0 };
};


} /* namespace instrumentation::detail */

namespace instrumentation {


class gauge {
  friend detail::gauge_impl;
  template<typename... LabelTypes> friend class gauge_vector;

  public:
  gauge() = default;

  void operator++() const noexcept;
  void operator++(int) const noexcept;
  void operator--() const noexcept;
  void operator--(int) const noexcept;
  void operator+=(double d) const noexcept;
  void operator-=(double d) const noexcept;
  void operator=(double d) const noexcept;

  explicit operator bool() const noexcept;
  auto operator!() const noexcept -> bool;
  auto operator*() const -> double;

  private:
  std::shared_ptr<detail::gauge_impl> impl_;
};


template<typename... LabelTypes>
class gauge_vector {
  private:
  using group_type = detail::metric_group<detail::gauge_impl, LabelTypes...>;

  public:
  gauge_vector() noexcept = default;
  gauge_vector(metric_name name, std::array<std::string, sizeof...(LabelTypes)> labels, std::string description = "");
  gauge_vector(engine& e, metric_name name, std::array<std::string, sizeof...(LabelTypes)> labels, std::string description = "");
  gauge_vector(std::string_view name, std::array<std::string, sizeof...(LabelTypes)> labels, std::string description = "");
  gauge_vector(engine& e, std::string_view name, std::array<std::string, sizeof...(LabelTypes)> labels, std::string description = "");

  auto labels(const LabelTypes&... values) const -> gauge;

  explicit operator bool() const noexcept;
  auto operator!() const noexcept -> bool;

  private:
  std::shared_ptr<group_type> impl_;
};


extern template class gauge_vector<>;


} /* namespace instrumentation */

#include "gauge-inl.h"

#endif /* INSTRUMENTATION_GAUGE_H */
