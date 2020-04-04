#ifndef INSTRUMENTATION_DETAIL_METRIC_GROUP_H
#define INSTRUMENTATION_DETAIL_METRIC_GROUP_H

#include <instrumentation/metric_name.h>
#include <instrumentation/tags.h>
#include <instrumentation/collector.h>
#include <cstddef>
#include <memory>
#include <array>
#include <string>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

namespace instrumentation::detail {


class metric_group_intf {
  public:
  metric_group_intf(const metric_group_intf&) = delete;
  metric_group_intf(metric_group_intf&&) = delete;
  metric_group_intf& operator=(const metric_group_intf&) = delete;
  metric_group_intf& operator=(metric_group_intf&&) = delete;

  virtual ~metric_group_intf() noexcept = default;

  virtual void collect(const metric_name& name, collector& c) const = 0;
};


template<typename MetricType, typename... LabelTypes>
class metric_group
: public metric_group_intf
{
  public:
  using metric_type = MetricType;
  using label_set = std::tuple<LabelTypes...>;
  static inline constexpr std::size_t NUM_LABELS = sizeof...(LabelTypes);

  protected:
  using metrics_map = std::unordered_map<label_set, std::shared_ptr<MetricType>>;

  public:
  template<typename... MetricArgs>
  static auto make(std::array<std::string, NUM_LABELS> label_names, std::string description, MetricArgs&&... metric_args) -> std::shared_ptr<metric_group>;

  metric_group(std::array<std::string, NUM_LABELS> label_names, std::string description);
  ~metric_group() noexcept override = default;

  void collect(const metric_name& name, collector& c) const override final;
  auto get(const label_set& labels) -> std::shared_ptr<metric_type> override final;

  private:
  auto get_existing_(const label_set& labels) const -> std::shared_ptr<metric_type>;
  virtual auto get_or_create_(const label_set& labels) -> std::shared_ptr<metric_type> = 0;

  auto make_tags_(const label_set& labels, std::index_sequence<> indices [[maybe_unused]]) const -> tags;
  template<std::size_t Idx0, std::size_t... Idx>
  auto make_tags_(const label_set& labels, std::index_sequence<Idx0, Idx...> indices [[maybe_unused]]) -> tags;

  protected:
  metrics_map metrics_;
  std::array<std::string, NUM_LABELS> label_names_;
  std::string description_;
  mutable std::shared_mutex mtx_;
};


template<typename MetricType, typename MetricConstructorArgTpl, typename... LabelTypes>
class metric_group_impl
: public metric_group<MetricType, LabelTypes...>,
  public std::enable_shared_from_this<metric_group<MetricType>>
{
  public:
  using metric_type = typename metric_group<MetricType, LabelTypes...>::metric_type;
  using label_set = typename metric_group<MetricType, LabelTypes...>::label_set;

  protected:
  using metrics_map = typename metric_group<MetricType, LabelTypes...>::metrics_map;

  public:
  template<typename... Args>
  explicit metric_group_impl(MetricConstructorArgTpl metric_constructor_arg_tpl, Args&&... args)
  : metric_group<MetricType, LabelTypes...>(std::forward<Args>(args)...),
    metric_constructor_arg_tpl_(std::move(metric_constructor_arg_tpl))
  {}

  private:
  auto get_or_create_(const label_set& labels) -> std::shared_ptr<metric_type> override final;

  const MetricConstructorArgTpl metric_constructor_arg_tpl_;
};


template<typename MetricType, typename... LabelTypes>
template<typename... MetricArgs>
auto metric_group<MetricType, LabelTypes...>::make(std::array<std::string, NUM_LABELS> label_names, std::string description, MetricArgs&&... metric_args) -> std::shared_ptr<metric_group> {
  using impl = metric_group_impl<MetricType, std::tuple<std::decay_t<MetricArgs>...>, LabelTypes...>;

  return std::make_shared<impl>(std::forward_as_tuple(std::forward<MetricArgs>(metric_args)...), std::move(label_names), std::move(description));
}

template<typename MetricType, typename... LabelTypes>
metric_group<MetricType, LabelTypes...>::metric_group(std::array<std::string, NUM_LABELS> label_names, std::string description)
: label_names_(std::move(label_names)),
  description_(std::move(description))
{}

template<typename MetricType, typename... LabelTypes>
void metric_group<MetricType, LabelTypes...>::collect(const metric_name& name, collector& c) const {
  const std::shared_lock<std::shared_mutex> lck{ mtx_ };

  c.visit_description(name, description_);

  for (const auto& tagged_metric : metrics_) {
    const auto& tags = make_tags_(tagged_metric.first, std::index_sequence_for<LabelTypes...>());
    const auto& metric = tagged_metric.second;

    metric.collect(name, tags, c);
  }
}

template<typename MetricType, typename... LabelTypes>
auto metric_group<MetricType, LabelTypes...>::get(const label_set& labels) -> std::shared_ptr<metric_type> {
  auto m = get_existing_(labels);
  if (m == nullptr) m = get_or_create_(labels);
  return m;
}

template<typename MetricType, typename... LabelTypes>
auto metric_group<MetricType, LabelTypes...>::get_existing_(const label_set& labels) const -> std::shared_ptr<metric_type> {
  const std::shared_lock<std::shared_mutex> lck{ mtx_ };

  auto iter = metrics_.find(labels);
  if (iter == metrics_.end()) return nullptr;
  return iter->second;
}

template<typename MetricType, typename... LabelTypes>
auto metric_group<MetricType, LabelTypes...>::make_tags_(const label_set& labels [[maybe_unused]], std::index_sequence<> indices [[maybe_unused]]) const -> tags {
  return tags();
}

template<typename MetricType, typename... LabelTypes>
template<std::size_t Idx0, std::size_t... Idx>
auto metric_group<MetricType, LabelTypes...>::make_tags_(const label_set& labels, std::index_sequence<Idx0, Idx...> indices [[maybe_unused]]) -> tags {
  return make_tags_(labels, std::index_sequence<Idx...>())
      .with(std::get<Idx0>(label_names_), std::get<Idx0>(labels));
}


template<typename MetricType, typename MetricConstructorArgTpl, typename... LabelTypes>
auto metric_group_impl<MetricType, MetricConstructorArgTpl, LabelTypes...>::get_or_create_(const label_set& labels) -> std::shared_ptr<metric_type> {
  auto new_metric = std::apply(
      [](const auto&... args) {
        return std::make_shared<metric_type>(args...);
      },
      metric_constructor_arg_tpl_);

  const std::lock_guard<std::shared_mutex> lck{ this->mtx_ };

  // Emplace will either create the element, or return the iterator to an existing element.
  const auto iter = std::get<0>(this->metrics_.emplace(labels, std::move(new_metric)));
  return iter->second;
}


} /* namespace instrumentation::detail */

#endif /* INSTRUMENTATION_DETAIL_METRIC_GROUP_H */
