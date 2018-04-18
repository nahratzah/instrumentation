#ifndef INSTRUMENTATION_TIMING_ACCUMULATE_H
#define INSTRUMENTATION_TIMING_ACCUMULATE_H

#include <instrumentation/instrumentation_export_.h>
#include <instrumentation/basic_metric.h>
#include <instrumentation/time_track.h>
#include <chrono>
#include <cstddef>

namespace instrumentation {


class instrumentation_export_ timing_accumulate final
: public basic_metric
{
 public:
  using clock_type = std::chrono::high_resolution_clock;
  using duration = clock_type::duration;

  template<std::size_t N = 0>
  timing_accumulate(std::string_view local_name, group& parent, const tag_map& t = {}) noexcept
  : basic_metric(local_name, parent, t)
  {
    this->enable();
  }

  ~timing_accumulate() noexcept override;

  auto visit(visitor& v) const -> void override;

  auto add(duration d)
  noexcept
  -> void {
    ticks_.fetch_add(d.count(), std::memory_order_relaxed);
  }

  auto operator*() const noexcept
  -> duration {
    return duration(ticks_.load(std::memory_order_relaxed));
  }

  template<typename Fn, typename... Args>
  auto measure(Fn&& fn, Args&&... args)
  -> decltype(std::invoke(std::declval<Fn>(), std::declval<Args>()...));

 private:
  std::atomic<duration::rep> ticks_;
};

template<typename Fn, typename... Args>
auto timing_accumulate::measure(Fn&& fn, Args&&... args)
-> decltype(std::invoke(std::declval<Fn>(), std::declval<Args>()...)) {
  time_track<timing_accumulate> tt{ *this };
  return std::invoke(std::forward<Fn>(fn), std::forward<Args>(args)...);
}


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_TIMING_ACCUMULATE_H */
