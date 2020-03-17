#include <instrumentation/engine.h>

namespace instrumentation {


engine_intf::~engine_intf() noexcept = default;


auto engine::new_counter(path p, tags t) const -> counter {
  if (!impl_) return {};
  return counter(impl_->new_counter(std::move(p), std::move(t)));
}

auto engine::new_gauge(path p, tags t) const -> gauge {
  if (!impl_) return {};
  return gauge(impl_->new_gauge(std::move(p), std::move(t)));
}

auto engine::new_string(path p, tags t) const -> string {
  if (!impl_) return {};
  return string(impl_->new_string(std::move(p), std::move(t)));
}

auto engine::new_timing(path p, tags t, timing::duration resolution, std::size_t buckets) const -> timing {
  if (!impl_) return {};
  return timing(impl_->new_timing(std::move(p), std::move(t), resolution, buckets));
}

auto engine::new_timing(path p, timing::duration resolution, std::size_t buckets) const -> timing {
  return new_timing(std::move(p), {}, resolution, buckets);
}

auto engine::new_cumulative_timing(path p, tags t) const -> timing {
  if (!impl_) return {};
  return timing(impl_->new_cumulative_timing(std::move(p), std::move(t)));
}

auto engine::new_counter_cb(path p, tags t, std::function<double()> cb) const -> std::shared_ptr<void> {
  if (!impl_ || !cb) return nullptr;
  return impl_->new_counter_cb(std::move(p), std::move(t), std::move(cb));
}

auto engine::new_counter_cb(path p, std::function<double()> cb) const -> std::shared_ptr<void> {
  return new_counter_cb(std::move(p), tags(), std::move(cb));
}

auto engine::new_gauge_cb(path p, tags t, std::function<double()> cb) const -> std::shared_ptr<void> {
  if (!impl_ || !cb) return nullptr;
  return impl_->new_gauge_cb(std::move(p), std::move(t), std::move(cb));
}

auto engine::new_gauge_cb(path p, std::function<double()> cb) const -> std::shared_ptr<void> {
  return new_gauge_cb(std::move(p), tags(), std::move(cb));
}

auto engine::new_string_cb(path p, tags t, std::function<std::string()> cb) const -> std::shared_ptr<void> {
  if (!impl_ || !cb) return nullptr;
  return impl_->new_string_cb(std::move(p), std::move(t), std::move(cb));
}

auto engine::new_string_cb(path p, std::function<std::string()> cb) const -> std::shared_ptr<void> {
  return new_string_cb(std::move(p), tags(), std::move(cb));
}

auto engine::global() -> engine& {
  static engine impl_;
  return impl_;
}


} /* namespace instrumentation */