#ifndef INSTRUMENTATION_TIMING_INL_H
#define INSTRUMENTATION_TIMING_INL_H

#include <instrumentation/engine.h>
#include <instrumentation/collector.h>

namespace instrumentation {


inline void timing::operator<<(duration d) const noexcept {
  if (impl_) impl_->inc(d);
}

inline auto timing::get_histogram() const -> std::tuple<std::vector<histogram_entry>, std::uint64_t> {
  if (impl_) impl_->get_histogram();
  return std::make_tuple(std::vector<histogram_entry>(), 0);
}


template<typename... LabelTypes>
timing_vector<LabelTypes...>::timing_vector(
    metric_name name,
    std::array<std::string, sizeof...(LabelTypes)> labels,
    std::string description)
: timing_vector(std::move(name), std::move(labels), default_buckets(), std::move(description))
{}

template<typename... LabelTypes>
timing_vector<LabelTypes...>::timing_vector(
    metric_name name,
    std::array<std::string, sizeof...(LabelTypes)> labels,
    std::vector<duration> buckets,
    std::string description)
: timing_vector(engine::global(), std::move(name), std::move(labels), std::move(buckets), std::move(description))
{}

template<typename... LabelTypes>
timing_vector<LabelTypes...>::timing_vector(
    engine& e,
    metric_name name,
    std::array<std::string, sizeof...(LabelTypes)> labels,
    std::string description)
: timing_vector(e, std::move(name), std::move(labels), default_buckets(), std::move(description))
{}

template<typename... LabelTypes>
timing_vector<LabelTypes...>::timing_vector(
    engine& e,
    metric_name name,
    std::array<std::string, sizeof...(LabelTypes)> labels,
    std::vector<duration> buckets,
    std::string description) {
  const auto raw_metric = e.get_metric(
      std::move(name),
      [&labels, &description, &buckets]() {
        return group_type::make(std::move(labels), std::move(description), std::move(buckets));
      });
  // We silently allow for a null impl if the metric doesn't match type.
  impl_ = std::dynamic_pointer_cast<group_type>(raw_metric);
}

template<typename... LabelTypes>
auto timing_vector<LabelTypes...>::labels(const LabelTypes&... values) const -> timing {
  timing result;
  if (impl_ == nullptr) return result;

  result.impl_ = impl_->get(std::make_tuple(values...));
  return result;
}


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_TIMING_INL_H */
