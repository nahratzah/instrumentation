#ifndef INSTRUMENTATION_TIMING_H
#define INSTRUMENTATION_TIMING_H

#include <instrumentation/fwd.h>
#include <instrumentation/detail/metric_group.h>
#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
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
  static auto default_buckets() -> std::vector<duration>;

  explicit timing_impl(const std::vector<duration>& thresholds);

  void inc(duration d, std::uint64_t v = 1) noexcept;
  auto get_histogram() const -> std::tuple<std::vector<histogram_entry>, std::uint64_t>;
  void collect(const metric_name& name, const tags& tags, collector& c);

  private:
  bucket_vector v_;
  std::atomic<std::uint64_t> large_v_{ 0u };
};


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
  void operator<<(duration d) const noexcept;

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
  timing_vector(metric_name name, std::array<std::string, sizeof...(LabelTypes)> labels, std::vector<duration> buckets, std::string description = "");
  timing_vector(engine& e, metric_name name, std::array<std::string, sizeof...(LabelTypes)> labels, std::vector<duration> buckets, std::string description = "");

  auto labels(const LabelTypes&... values) const -> timing;

  private:
  std::shared_ptr<group_type> impl_;
};


extern template class timing_vector<>;


} /* namespace instrumentation */

#include "timing-inl.h"

#endif /* INSTRUMENTATION_TIMING_H */
