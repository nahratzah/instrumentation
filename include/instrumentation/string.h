#ifndef INSTRUMENTATION_STRING_H
#define INSTRUMENTATION_STRING_H

#include <instrumentation/fwd.h>
#include <instrumentation/detail/metric_group.h>
#include <array>
#include <atomic>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>

namespace instrumentation::detail {


class string_impl
: public std::enable_shared_from_this<string_impl>
{
  public:
  void set(std::string s);
  auto get() const -> std::string;
  void collect(const metric_name& name, const tags& tags, collector& c);

  private:
  mutable std::shared_mutex mtx_;
  std::string v_;
};


} /* namespace instrumentation */

namespace instrumentation {


class string {
  friend detail::string_impl;
  template<typename... LabelTypes> friend class string_vector;

  public:
  string() = default;

  void operator=(std::string s) const;

  explicit operator bool() const noexcept;
  auto operator!() const noexcept -> bool;
  auto operator*() const -> std::string;

  private:
  std::shared_ptr<detail::string_impl> impl_;
};


template<typename... LabelTypes>
class string_vector {
  private:
  using group_type = detail::metric_group<detail::string_impl, LabelTypes...>;

  public:
  string_vector() noexcept = default;
  string_vector(metric_name name, std::array<std::string, sizeof...(LabelTypes)> labels, std::string description = "");
  string_vector(engine& e, metric_name name, std::array<std::string, sizeof...(LabelTypes)> labels, std::string description = "");

  auto labels(const LabelTypes&... values) const -> string;

  private:
  std::shared_ptr<group_type> impl_;
};


extern template class string_vector<>;


} /* namespace instrumentation */

#include "string-inl.h"

#endif /* INSTRUMENTATION_STRING_H */
