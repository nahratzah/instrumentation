#ifndef INSTRUMENTATION_TIMING_ACCUMULATE_H
#define INSTRUMENTATION_TIMING_ACCUMULATE_H

#include <instrumentation/instrumentation_export_.h>
#include <instrumentation/hierarchy.h>
#include <chrono>

namespace instrumentation {


class instrumentation_export_ timing_accumulate final
: public hierarchy
{
 public:
  using duration = std::chrono::high_resolution_clock::duration;

  using hierarchy::hierarchy;

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

 private:
  std::atomic<duration::rep> ticks_;
};


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_TIMING_ACCUMULATE_H */
