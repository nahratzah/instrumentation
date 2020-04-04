#ifndef INSTRUMENTATION_GAUGE_INL_H
#define INSTRUMENTATION_GAUGE_INL_H

#include <instrumentation/engine.h>
#include <instrumentation/collector.h>

namespace instrumentation {


inline void gauge::operator++() const noexcept {
  if (impl_) impl_->inc();
}

inline void gauge::operator++(int) const noexcept {
  if (impl_) impl_->inc();
}

inline void gauge::operator--() const noexcept {
  if (impl_) impl_->dec();
}

inline void gauge::operator--(int) const noexcept {
  if (impl_) impl_->dec();
}

inline void gauge::operator+=(double d) const noexcept {
  if (impl_) impl_->inc(d);
}

inline void gauge::operator-=(double d) const noexcept {
  if (impl_) impl_->dec(d);
}

inline void gauge::operator=(double d) const noexcept {
  if (impl_) impl_->set(d);
}

inline gauge::operator bool() const noexcept {
  return impl_ != nullptr;
}

inline auto gauge::operator!() const noexcept -> bool {
  return impl_ == nullptr;
}

inline auto gauge::operator*() const -> double {
  if (!impl_) return 0.0;
  return impl_->get();
}


template<typename... LabelTypes>
gauge_vector<LabelTypes...>::gauge_vector(
    metric_name name,
    std::array<std::string, sizeof...(LabelTypes)> labels,
    std::string description)
: gauge_vector(engine::global(), std::move(name), std::move(labels), std::move(description))
{}

template<typename... LabelTypes>
gauge_vector<LabelTypes...>::gauge_vector(
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
auto gauge_vector<LabelTypes...>::labels(const LabelTypes&... values) const -> gauge {
  gauge result;
  if (impl_ == nullptr) return result;

  result.impl_ = impl_->get(std::make_tuple(values...));
  return result;
}


} /* namespace instrumentation */

namespace instrumentation::detail {


inline void gauge_impl::inc(double d) noexcept {
  double expect = v_.load(std::memory_order_relaxed);
  while (!v_.compare_exchange_weak(expect, expect + d, std::memory_order_relaxed, std::memory_order_relaxed)) {
    // SKIP
  }
}

inline void gauge_impl::dec(double d) noexcept {
  double expect = v_.load(std::memory_order_relaxed);
  while (!v_.compare_exchange_weak(expect, expect - d, std::memory_order_relaxed, std::memory_order_relaxed)) {
    // SKIP
  }
}

inline void gauge_impl::set(double d) noexcept {
  v_.store(d, std::memory_order_relaxed);
}

inline auto gauge_impl::get() const noexcept -> double {
  return v_.load(std::memory_order_relaxed);
}

inline void gauge_impl::collect(const metric_name& name, const tags& tags, collector& c) {
  gauge tmp;
  tmp.impl_ = shared_from_this();
  return c.visit(name, tags, tmp);
}


} /* namespace instrumentation::detail */

#endif /* INSTRUMENTATION_GAUGE_INL_H */
