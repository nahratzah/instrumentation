#ifndef INSTRUMENTATION_ENGINE_H
#define INSTRUMENTATION_ENGINE_H

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <instrumentation/path.h>
#include <instrumentation/tags.h>
#include <instrumentation/counter.h>
#include <instrumentation/gauge.h>
#include <instrumentation/string.h>
#include <instrumentation/timing.h>

namespace instrumentation {


class engine_intf {
  public:
  virtual ~engine_intf() noexcept;

  virtual auto new_counter(path p, tags t) -> std::shared_ptr<counter_intf> = 0;
  virtual auto new_gauge(path p, tags t) -> std::shared_ptr<gauge_intf> = 0;
  virtual auto new_string(path p, tags t) -> std::shared_ptr<string_intf> = 0;
  virtual auto new_timing(path p, tags t, timing_intf::duration resolution, std::size_t buckets) -> std::shared_ptr<timing_intf> = 0;
  virtual auto new_cumulative_timing(path p, tags t) -> std::shared_ptr<timing_intf> = 0;

  virtual auto new_counter_cb(path p, tags t, std::function<double()> cb) -> std::shared_ptr<void> = 0;
  virtual auto new_gauge_cb(path p, tags t, std::function<double()> cb) -> std::shared_ptr<void> = 0;
  virtual auto new_string_cb(path p, tags t, std::function<std::string()> cb) -> std::shared_ptr<void> = 0;
};


class engine {
  public:
  engine() = default;

  engine(std::shared_ptr<engine_intf> impl) noexcept;

  auto new_counter(path p, tags t = tags()) const -> counter;
  auto new_gauge(path p, tags t = tags()) const -> gauge;
  auto new_string(path p, tags t = tags()) const -> string;
  auto new_timing(path p, tags t, timing::duration resolution = timing::dfl_resolution, std::size_t buckets = timing::dfl_buckets) const -> timing;
  auto new_timing(path p, timing::duration resolution = timing::dfl_resolution, std::size_t buckets = timing::dfl_buckets) const -> timing;
  auto new_cumulative_timing(path p, tags t) const -> timing;

  auto new_counter_cb(path p, tags t, std::function<double()> cb) const -> std::shared_ptr<void>;
  auto new_counter_cb(path p, std::function<double()> cb) const -> std::shared_ptr<void>;
  auto new_gauge_cb(path p, tags t, std::function<double()> cb) const -> std::shared_ptr<void>;
  auto new_gauge_cb(path p, std::function<double()> cb) const -> std::shared_ptr<void>;
  auto new_string_cb(path p, tags t, std::function<std::string()> cb) const -> std::shared_ptr<void>;
  auto new_string_cb(path p, std::function<std::string()> cb) const -> std::shared_ptr<void>;

  static auto global() -> engine&;

  private:
  std::shared_ptr<engine_intf> impl_;
};


inline engine::engine(std::shared_ptr<engine_intf> impl) noexcept
: impl_(impl)
{}


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_ENGINE_H */
