#ifndef INSTRUMENTATION_TIMING_H
#define INSTRUMENTATION_TIMING_H

#include <instrumentation/fwd.h>
#include <instrumentation/detail/metric_group.h>
#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <iosfwd>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace instrumentation::detail {


class timing_impl
: public std::enable_shared_from_this<timing_impl>
{
  public:
  using clock_type = std::chrono::high_resolution_clock;
  using duration = clock_type::duration;

  struct histogram_entry {
    duration le;
    std::uint64_t bucket_count;
  };

  private:
  struct bucket {
    duration::rep le;
    std::atomic<std::uint64_t> v{ 0u };
  };

  struct search_comparison;

  using bucket_vector = std::vector<bucket>;

  public:
  instrumentation_export_
  explicit timing_impl(const std::vector<duration>& thresholds);

  instrumentation_export_
  void inc(duration d, std::uint64_t v = 1) noexcept;
  instrumentation_export_
  auto get_histogram() const -> std::tuple<std::vector<histogram_entry>, std::uint64_t>;
  void collect(const metric_name& name, const tags& tags, collector& c);

  instrumentation_export_
  static auto default_buckets() -> std::vector<duration>;

  private:
  bucket_vector v_;
  std::atomic<std::uint64_t> large_v_{ 0u };
};

auto operator==(const timing_impl::histogram_entry& x, const timing_impl::histogram_entry& y) noexcept -> bool;
auto operator!=(const timing_impl::histogram_entry& x, const timing_impl::histogram_entry& y) noexcept -> bool;

template<typename Char, typename Traits>
auto operator<<(std::basic_ostream<Char, Traits>& out, const timing_impl::histogram_entry& entry) -> std::basic_ostream<Char, Traits>&;


} /* namespace instrumentation::detail */

namespace instrumentation {


class timing {
  friend detail::timing_impl;
  template<typename... LabelTypes> friend class timing_vector;

  public:
  using clock_type = detail::timing_impl::clock_type;
  using duration = detail::timing_impl::duration;
  using histogram_entry = detail::timing_impl::histogram_entry;

  public:
  auto operator<<(duration d) const noexcept -> const timing&;

  explicit operator bool() const noexcept;
  auto operator!() const noexcept -> bool;
  auto operator*() const -> std::tuple<std::vector<histogram_entry>, std::uint64_t>;

  private:
  std::shared_ptr<detail::timing_impl> impl_;
};


template<typename... LabelTypes>
class timing_vector {
  private:
  using group_type = detail::metric_group<detail::timing_impl, LabelTypes...>;

  public:
  using clock_type = timing::clock_type;
  using duration = timing::duration;

  static auto default_buckets() -> std::vector<duration> { return detail::timing_impl::default_buckets(); }

  timing_vector() noexcept = default;
  timing_vector(metric_name name, std::array<std::string, sizeof...(LabelTypes)> labels, std::string description = "");
  timing_vector(engine& e, metric_name name, std::array<std::string, sizeof...(LabelTypes)> labels, std::string description = "");
  timing_vector(metric_name name, std::array<std::string, sizeof...(LabelTypes)> labels, std::vector<duration> buckets, std::string description);
  timing_vector(engine& e, metric_name name, std::array<std::string, sizeof...(LabelTypes)> labels, std::vector<duration> buckets, std::string description);

  timing_vector(std::string_view name, std::array<std::string, sizeof...(LabelTypes)> labels, std::string description = "");
  timing_vector(engine& e, std::string_view name, std::array<std::string, sizeof...(LabelTypes)> labels, std::string description = "");
  timing_vector(std::string_view name, std::array<std::string, sizeof...(LabelTypes)> labels, std::vector<duration> buckets, std::string description);
  timing_vector(engine& e, std::string_view name, std::array<std::string, sizeof...(LabelTypes)> labels, std::vector<duration> buckets, std::string description);

  auto labels(const LabelTypes&... values) const -> timing;

  explicit operator bool() const noexcept;
  auto operator!() const noexcept -> bool;

  private:
  std::shared_ptr<group_type> impl_;
};


} /* namespace instrumentation */

#include "timing-inl.h"

#endif /* INSTRUMENTATION_TIMING_H */
