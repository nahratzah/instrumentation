#ifndef INSTRUMENTATION_PROMETHEUS_ENGINE_H
#define INSTRUMENTATION_PROMETHEUS_ENGINE_H

#include <instrumentation/engine.h>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace instrumentation {


class prometheus_engine
: public std::enable_shared_from_this<prometheus_engine>,
  public engine_intf
{
  private:
  class collector_intf;
  class metric_collector;
  class timing_collector;
  class cumulative_timing_collector;
  class functor_collector;

  public:
  enum class prom_type {
    untyped,
    counter,
    gauge,
    histogram,
    summary
  };

  private:
  struct metric_name {
    std::string name, tags;

    metric_name(std::string name, std::string tags) noexcept
    : name(std::move(name)),
      tags(std::move(tags))
    {}
  };

  ///\brief Specialized hasher that only compares the name for equality, but not the tags.
  ///\details We make use that the unordered_multimap requires grouping for equality-compared items.
  struct metric_name_hasher {
    auto operator()(const metric_name& m) const noexcept -> std::size_t {
      return sv_hash_(m.name);
    }

    private:
    std::hash<std::string_view> sv_hash_;
  };

  ///\brief Specialized hasher that only compares the name for equality, but not the tags.
  ///\details We make use that the unordered_multimap requires grouping for equality-compared items.
  struct metric_name_eq {
    auto operator()(const metric_name& x, const metric_name& y) const noexcept -> bool {
      return x.name == y.name;
    }
  };

  using help_type = std::unordered_map<std::string, std::string>;
  using map_type = std::unordered_multimap<metric_name, std::weak_ptr<collector_intf>, metric_name_hasher, metric_name_eq>;

  public:
  static auto fix_prom_name(std::string_view s) -> std::string;
  static auto quote_string(std::string_view s) -> std::string;
  static auto path_to_string(const path& p) -> std::string;
  static auto tags_to_string(const tags& t) -> std::string;

  prometheus_engine() = default;

  void add_help(const path& p, std::string help);

  auto new_counter(path p, tags t) -> std::shared_ptr<counter_intf> override;
  auto new_gauge(path p, tags t) -> std::shared_ptr<gauge_intf> override;
  auto new_string(path p, tags t) -> std::shared_ptr<string_intf> override;
  auto new_timing(path p, tags t, timing_intf::duration resolution, std::size_t buckets) -> std::shared_ptr<timing_intf> override;
  auto new_cumulative_timing(path p, tags t) -> std::shared_ptr<timing_intf> override;

  auto new_counter_cb(path p, tags t, std::function<double()> cb) -> std::shared_ptr<void> override;
  auto new_gauge_cb(path p, tags t, std::function<double()> cb) -> std::shared_ptr<void> override;
  auto new_string_cb(path p, tags t, std::function<std::string()> cb) -> std::shared_ptr<void> override;

  void collect(std::ostream& out) const;
  auto collect() const -> std::string;
  void maintenance() const;

  private:
  mutable std::shared_mutex mtx_;
  help_type help_;
  mutable map_type map_;
};


auto operator<<(std::ostream& out, prometheus_engine::prom_type t) -> std::ostream&;


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_PROMETHEUS_ENGINE_H */
