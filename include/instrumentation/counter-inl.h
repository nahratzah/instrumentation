#ifndef INSTRUMENTATION_COUNTER_INL_H
#define INSTRUMENTATION_COUNTER_INL_H

#include <instrumentation/engine.h>
#include <instrumentation/collector.h>

namespace instrumentation {


inline void counter::operator++() const noexcept {
  if (impl_) impl_->inc();
}

inline void counter::operator++(int) const noexcept {
  if (impl_) impl_->inc();
}

inline void counter::operator+=(double d) const noexcept {
  if (impl_) impl_->inc(d);
}

inline counter::operator bool() const noexcept {
  return impl_ != nullptr;
}

inline auto counter::operator!() const noexcept -> bool {
  return impl_ == nullptr;
}

inline auto counter::operator*() const -> double {
  if (!impl_) return 0.0;
  return impl_->get();
}


template<typename... LabelTypes>
counter_vector<LabelTypes...>::counter_vector(
    metric_name name,
    std::array<std::string, sizeof...(LabelTypes)> labels,
    std::string description)
: counter_vector(engine::global(), std::move(name), std::move(labels), std::move(description))
{}

template<typename... LabelTypes>
counter_vector<LabelTypes...>::counter_vector(
    engine& e,
    metric_name name,
    std::array<std::string, sizeof...(LabelTypes)> labels,
    std::string description) {
  const auto raw_metric = e.get_metric(
      std::move(name),
      [&labels, &description]() {
        return group_type::make(std::move(labels), std::move(description));
      });
  // We silently allow for a null impl if the metric doesn't match type.
  impl_ = std::dynamic_pointer_cast<group_type>(raw_metric);
}

template<typename... LabelTypes>
counter_vector<LabelTypes...>::counter_vector(
    std::string_view name,
    std::array<std::string, sizeof...(LabelTypes)> labels,
    std::string description)
: counter_vector(metric_name(name), std::move(labels), std::move(description))
{}

template<typename... LabelTypes>
counter_vector<LabelTypes...>::counter_vector(
    engine& e,
    std::string_view name,
    std::array<std::string, sizeof...(LabelTypes)> labels,
    std::string description)
: counter_vector(e, metric_name(name), std::move(labels), std::move(description))
{}

template<typename... LabelTypes>
auto counter_vector<LabelTypes...>::labels(const LabelTypes&... values) const -> counter {
  counter result;
  if (impl_ == nullptr) return result;

  result.impl_ = impl_->get(std::make_tuple(values...));
  return result;
}

template<typename... LabelTypes>
counter_vector<LabelTypes...>::operator bool() const noexcept {
  return impl_ != nullptr;
}

template<typename... LabelTypes>
auto counter_vector<LabelTypes...>::operator!() const noexcept -> bool {
  return impl_ == nullptr;
}


} /* namespace instrumentation */

namespace instrumentation::detail {


inline void counter_impl::inc(double d) noexcept {
  double expect = v_.load(std::memory_order_relaxed);
  while (!v_.compare_exchange_weak(expect, expect + d, std::memory_order_relaxed, std::memory_order_relaxed)) {
    // SKIP
  }
}

inline auto counter_impl::get() const noexcept -> double {
  return v_.load(std::memory_order_relaxed);
}

inline void counter_impl::collect(const metric_name& name, const tags& tags, collector& c) {
  counter tmp;
  tmp.impl_ = shared_from_this();
  return c.visit(name, tags, tmp);
}


} /* namespace instrumentation::detail */

#endif /* INSTRUMENTATION_COUNTER_INL_H */
