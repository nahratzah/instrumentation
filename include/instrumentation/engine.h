#ifndef INSTRUMENTATION_ENGINE_H
#define INSTRUMENTATION_ENGINE_H

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <instrumentation/detail/export_.h>
#include <instrumentation/metric_name.h>
#include <instrumentation/tags.h>
#include <instrumentation/collector.h>
#include <instrumentation/detail/metric_group.h>

namespace instrumentation {


class engine {
  public:
  engine() = default;

  instrumentation_export_
  static auto global() -> engine&;

  instrumentation_export_
  void collect(collector& c) const;

  template<typename MetricCb>
  auto get_metric(metric_name name, MetricCb&& cb) -> std::shared_ptr<detail::metric_group_intf>;

  private:
  auto get_existing_(const metric_name& name) const -> std::shared_ptr<detail::metric_group_intf>;
  template<typename MetricCb>
  auto get_or_create_(metric_name&& name, MetricCb&& cb) -> std::shared_ptr<detail::metric_group_intf>;

  std::unordered_map<metric_name, std::shared_ptr<detail::metric_group_intf>> metrics_;
  mutable std::shared_mutex mtx_;
};


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
  return std::get<0>(metrics_.emplace(std::move(name), std::invoke(std::forward<MetricCb>(cb))))->second;
}


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_ENGINE_H */
