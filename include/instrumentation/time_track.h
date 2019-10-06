#ifndef INSTRUMENTATION_TIME_TRACK_H
#define INSTRUMENTATION_TIME_TRACK_H

#include <chrono>
#include <type_traits>
#include <functional>
#include <utility>

namespace instrumentation {


template<typename Metric>
class time_track {
 private:
  using clock_type = typename Metric::clock_type;
  using duration = typename Metric::duration;

 public:
  class paused {
   public:
    constexpr paused() noexcept
    : tt_(nullptr)
    {}

    paused(paused&& y)
    : tt_(std::exchange(y.tt_, nullptr))
    {}

    auto operator=(paused&& y)
    noexcept
    -> paused& {
      using std::swap;
      swap(tt_, y.tt_);
    }

    paused(time_track& tt) noexcept
    : tt_(&tt)
    {
      tt_->pause();
    }

    ~paused() noexcept {
      if (tt_ != nullptr)
        tt_->unpause();
    }

   private:
    time_track* tt_;
  };

  time_track(Metric& metric)
  : metric_(metric)
  {}

  time_track(const time_track&) = delete;
  time_track(time_track&&) = delete;

  ~time_track() noexcept {
    if (active_) pause();
    metric_ << inactive_;
  }

  auto pause() noexcept {
    if (active_) {
      typename clock_type::time_point end = clock_type::now();
      inactive_ += end - start_;
      active_ = false;
    }
  }

  auto unpause() noexcept {
    if (!active_) {
      start_ = clock_type::now();
      active_ = true;
    }
  }

  template<typename Fn, typename... Args>
  auto do_untracked(Fn&& fn, Args&&... args)
  -> decltype(std::invoke(std::declval<Fn>(), std::declval<Args>()...)) {
    paused p{ *this };
    return std::invoke(std::forward<Fn>(fn), std::forward<Args>(args)...);
  }

 private:
  Metric& metric_;
  typename clock_type::time_point start_ = clock_type::now();
  bool active_ = true;
  duration inactive_{0};
};


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_TIME_TRACK_H */
