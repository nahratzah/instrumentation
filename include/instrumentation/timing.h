#ifndef INSTRUMENTATION_TIMING_H
#define INSTRUMENTATION_TIMING_H

#include <chrono>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>
#include <instrumentation/tags.h>

namespace instrumentation {


class timing_intf {
  public:
  using clock_type = std::chrono::high_resolution_clock;
  using duration = clock_type::duration;

  protected:
  virtual ~timing_intf() noexcept;

  public:
  void add(duration d) noexcept;

  private:
  virtual void do_add(duration d) noexcept = 0;
};


class timing {
  public:
  using clock_type = timing_intf::clock_type;
  using duration = timing_intf::duration;

  static constexpr std::chrono::milliseconds dfl_resolution{1};
  static constexpr std::size_t dfl_buckets = std::chrono::seconds{1} / dfl_resolution;

  timing() = default;

  explicit timing(std::string_view name, duration resolution = dfl_resolution, std::size_t buckets = dfl_buckets);
  timing(std::string_view name, std::initializer_list<std::pair<const std::string, tags::tag_value>> tags, duration resolution = dfl_resolution, std::size_t buckets = dfl_buckets);
  timing(std::string_view name, instrumentation::tags tags, duration resolution = dfl_resolution, std::size_t buckets = dfl_buckets);

  static auto cumulative(std::string_view name, std::initializer_list<std::pair<const std::string, tags::tag_value>> tags = {}) -> timing;
  static auto cumulative(std::string_view name, instrumentation::tags tags) -> timing;

  timing(std::shared_ptr<timing_intf> impl) noexcept
  : impl_(std::move(impl))
  {}

  void operator<<(duration d) const noexcept;

  private:
  std::shared_ptr<timing_intf> impl_;
};


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_TIMING_H */
