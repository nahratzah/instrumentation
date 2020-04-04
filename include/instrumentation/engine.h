#ifndef INSTRUMENTATION_ENGINE_H
#define INSTRUMENTATION_ENGINE_H

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <instrumentation/metric_name.h>
#include <instrumentation/tags.h>
#include <instrumentation/string.h>
#include <instrumentation/timing.h>
#include <instrumentation/collector.h>
#include <instrumentation/detail/metric_group.h>

namespace instrumentation {


class engine_intf {
  public:
  virtual ~engine_intf() noexcept;

  virtual auto new_string(metric_name p, tags t) -> std::shared_ptr<string_intf> = 0;
  virtual auto new_timing(metric_name p, tags t, timing_intf::duration resolution, std::size_t buckets) -> std::shared_ptr<timing_intf> = 0;
  virtual auto new_cumulative_timing(metric_name p, tags t) -> std::shared_ptr<timing_intf> = 0;

  virtual auto new_counter_cb(metric_name p, tags t, std::function<double()> cb) -> std::shared_ptr<void> = 0;
  virtual auto new_gauge_cb(metric_name p, tags t, std::function<double()> cb) -> std::shared_ptr<void> = 0;
  virtual auto new_string_cb(metric_name p, tags t, std::function<std::string()> cb) -> std::shared_ptr<void> = 0;
};


class engine {
  public:
  engine() = default;

  engine(std::shared_ptr<engine_intf> impl) noexcept;

  [[deprecated]]
  auto new_string(metric_name p, tags t = tags()) const -> string;
  [[deprecated]]
  auto new_timing(metric_name p, tags t, timing::duration resolution = timing::dfl_resolution, std::size_t buckets = timing::dfl_buckets) const -> timing;
  [[deprecated]]
  auto new_timing(metric_name p, timing::duration resolution = timing::dfl_resolution, std::size_t buckets = timing::dfl_buckets) const -> timing;
  [[deprecated]]
  auto new_cumulative_timing(metric_name p, tags t) const -> timing;

  [[deprecated]]
  auto new_counter_cb(metric_name p, tags t, std::function<double()> cb) const -> std::shared_ptr<void>;
  [[deprecated]]
  auto new_counter_cb(metric_name p, std::function<double()> cb) const -> std::shared_ptr<void>;
  [[deprecated]]
  auto new_gauge_cb(metric_name p, tags t, std::function<double()> cb) const -> std::shared_ptr<void>;
  [[deprecated]]
  auto new_gauge_cb(metric_name p, std::function<double()> cb) const -> std::shared_ptr<void>;
  [[deprecated]]
  auto new_string_cb(metric_name p, tags t, std::function<std::string()> cb) const -> std::shared_ptr<void>;
  [[deprecated]]
  auto new_string_cb(metric_name p, std::function<std::string()> cb) const -> std::shared_ptr<void>;

  static auto global() -> engine&;

  void collect(collector& c) const;

  template<typename MetricCb>
  auto get_metric(metric_name name, MetricCb&& cb) -> std::shared_ptr<detail::metric_group_intf>;

  private:
  auto get_existing_(const metric_name& name) const -> std::shared_ptr<detail::metric_group_intf>;
  template<typename MetricCb>
  auto get_or_create_(metric_name&& name, MetricCb&& cb) -> std::shared_ptr<detail::metric_group_intf>;

  std::shared_ptr<engine_intf> impl_ [[deprecated]];
  std::unordered_map<metric_name, std::shared_ptr<detail::metric_group_intf>> metrics_;
  mutable std::shared_mutex mtx_;
};


inline engine::engine(std::shared_ptr<engine_intf> impl) noexcept
: impl_(impl)
{}


template<typename MetricCb>
inline auto engine::get_metric(metric_name name, MetricCb&& cb) -> std::shared_ptr<detail::metric_group_intf> {
  auto mg = get_existing_(name);
  if (mg == nullptr) mg = get_or_create_(std::move(name), std::forward<MetricCb>(cb));
  return mg;
}

inline auto engine::get_existing_(const metric_name& name) const -> std::shared_ptr<detail::metric_group_intf> {
  std::shared_lock<std::shared_mutex> lck{ mtx_ };

  auto iter = metrics_.find(name);
  if (iter == metrics_.end()) return nullptr;
  return iter->second;
}

template<typename MetricCb>
inline auto engine::get_or_create_(metric_name&& name, MetricCb&& cb) -> std::shared_ptr<detail::metric_group_intf> {
  std::lock_guard<std::shared_mutex> lck{ mtx_ };
  return std::get<0>(metrics_.emplace(std::move(name), std::invoke(std::forward<MetricCb>(cb))));
}


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_ENGINE_H */
