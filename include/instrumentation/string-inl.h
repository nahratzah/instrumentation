#ifndef INSTRUMENTATION_STRING_INL_H
#define INSTRUMENTATION_STRING_INL_H

#include <instrumentation/engine.h>
#include <instrumentation/collector.h>

namespace instrumentation {


inline void string::operator=(std::string s) const {
  if (impl_) impl_->set(std::move(s));
}

inline string::operator bool() const noexcept {
  return impl_ != nullptr;
}

inline auto string::operator!() const noexcept -> bool {
  return impl_ == nullptr;
}

inline auto string::operator*() const -> std::string {
  if (!impl_) return "";
  return impl_->get();
}


template<typename... LabelTypes>
string_vector<LabelTypes...>::string_vector(
    metric_name name,
    std::array<std::string, sizeof...(LabelTypes)> labels,
    std::string description)
: string_vector(engine::global(), std::move(name), std::move(labels), std::move(description))
{}

template<typename... LabelTypes>
string_vector<LabelTypes...>::string_vector(
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
string_vector<LabelTypes...>::string_vector(
    std::string_view name,
    std::array<std::string, sizeof...(LabelTypes)> labels,
    std::string description)
: string_vector(metric_name(name), std::move(labels), std::move(description))
{}

template<typename... LabelTypes>
string_vector<LabelTypes...>::string_vector(
    engine& e,
    std::string_view name,
    std::array<std::string, sizeof...(LabelTypes)> labels,
    std::string description)
: string_vector(e, metric_name(name), std::move(labels), std::move(description))
{}

template<typename... LabelTypes>
auto string_vector<LabelTypes...>::labels(const LabelTypes&... values) const -> string {
  string result;
  if (impl_ == nullptr) return result;

  result.impl_ = impl_->get(std::make_tuple(values...));
  return result;
}

template<typename... LabelTypes>
string_vector<LabelTypes...>::operator bool() const noexcept {
  return impl_ != nullptr;
}

template<typename... LabelTypes>
auto string_vector<LabelTypes...>::operator!() const noexcept -> bool {
  return impl_ == nullptr;
}


} /* namespace instrumentation */

namespace instrumentation::detail {


inline void string_impl::set(std::string s) {
  std::lock_guard<std::shared_mutex> lck{ mtx_ };
  v_ = std::move(s);
}

inline auto string_impl::get() const -> std::string {
  std::shared_lock<std::shared_mutex> lck{ mtx_ };
  return v_;
}

inline void string_impl::collect(const metric_name& name, const tags& tags, collector& c) {
  string tmp;
  tmp.impl_ = shared_from_this();
  return c.visit(name, tags, tmp);
}


} /* namespace instrumentation::detail */

#endif /* INSTRUMENTATION_STRING_INL_H */
