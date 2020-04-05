#ifndef INSTRUMENTATION_TIMING_INL_H
#define INSTRUMENTATION_TIMING_INL_H

#include <instrumentation/engine.h>
#include <instrumentation/collector.h>
#include <string_view>

namespace instrumentation {


inline auto timing::operator<<(duration d) const noexcept -> const timing& {
  if (impl_) impl_->inc(d);
  return *this;
}

inline timing::operator bool() const noexcept {
  return impl_ != nullptr;
}

inline auto timing::operator!() const noexcept -> bool {
  return impl_ == nullptr;
}

inline auto timing::operator*() const -> std::tuple<std::vector<histogram_entry>, std::uint64_t> {
  if (impl_) return impl_->get_histogram();
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
timing_vector<LabelTypes...>::timing_vector(
    std::string_view name,
    std::array<std::string, sizeof...(LabelTypes)> labels,
    std::string description)
: timing_vector(metric_name(name), std::move(labels), std::move(description))
{}

template<typename... LabelTypes>
timing_vector<LabelTypes...>::timing_vector(
    engine& e,
    std::string_view name,
    std::array<std::string, sizeof...(LabelTypes)> labels,
    std::string description)
: timing_vector(e, metric_name(name), std::move(labels), std::move(description))
{}

template<typename... LabelTypes>
timing_vector<LabelTypes...>::timing_vector(
    std::string_view name,
    std::array<std::string, sizeof...(LabelTypes)> labels,
    std::vector<duration> buckets,
    std::string description)
: timing_vector(metric_name(name), std::move(labels), std::move(buckets), std::move(description))
{}

template<typename... LabelTypes>
timing_vector<LabelTypes...>::timing_vector(
    engine& e,
    std::string_view name,
    std::array<std::string, sizeof...(LabelTypes)> labels,
    std::vector<duration> buckets,
    std::string description)
: timing_vector(e, metric_name(name), std::move(labels), std::move(buckets), std::move(description))
{}

template<typename... LabelTypes>
auto timing_vector<LabelTypes...>::labels(const LabelTypes&... values) const -> timing {
  timing result;
  if (impl_ == nullptr) return result;

  result.impl_ = impl_->get(std::make_tuple(values...));
  return result;
}

template<typename... LabelTypes>
timing_vector<LabelTypes...>::operator bool() const noexcept {
  return impl_ != nullptr;
}

template<typename... LabelTypes>
auto timing_vector<LabelTypes...>::operator!() const noexcept -> bool {
  return impl_ == nullptr;
}


} /* namespace instrumentation */

namespace instrumentation::detail {


inline void timing_impl::collect(const metric_name& name, const tags& tags, collector& c) {
  timing tmp;
  tmp.impl_ = shared_from_this();
  return c.visit(name, tags, tmp);
}

inline auto operator==(const timing_impl::histogram_entry& x, const timing_impl::histogram_entry& y) noexcept -> bool {
  return x.le == y.le && x.bucket_count == y.bucket_count;
}

inline auto operator!=(const timing_impl::histogram_entry& x, const timing_impl::histogram_entry& y) noexcept -> bool {
  return !(x == y);
}

template<typename Char, typename Traits, typename = void>
struct test_writeable_duration_
: std::false_type
{};

template<typename Char, typename Traits>
struct test_writeable_duration_<
    Char, Traits,
    std::void_t<decltype(std::declval<std::basic_ostream<Char, Traits>&>() << std::declval<const timing_impl::duration&>())>>
: std::true_type
{};

template<typename Char, typename Traits>
using test_writeable_duration = typename test_writeable_duration_<Char, Traits>::type;
template<typename Char, typename Traits>
inline constexpr bool test_writeable_duration_v = test_writeable_duration<Char, Traits>::value;

template<typename Char, typename Traits>
auto operator<<(std::basic_ostream<Char, Traits>& out, const timing_impl::histogram_entry& entry) -> std::basic_ostream<Char, Traits>& {
  using namespace std::string_view_literals;

  for (char c : "timing::histogram_entry{le="sv) out.put(out.widen(c));

  // C++20 allows us to write durations directly.
  // Test if we can make use of that here.
  if constexpr(test_writeable_duration_v<Char, Traits>) {
    out << entry.le;
  } else {
    // We can't use the builtin.
    out << std::chrono::duration<double>(entry.le).count();
    out.put(out.widen('s'));
  }

  for (char c : ", "sv) out.put(out.widen(c));
  out << entry.bucket_count;
  out.put(out.widen('}'));

  return out;
}


} /* namespace instrumentation::detail */

#endif /* INSTRUMENTATION_TIMING_INL_H */
