#include <instrumentation/timing.h>
#include <algorithm>
#include <iterator>
#include <stdexcept>

namespace instrumentation::detail {

struct timing_impl::search_comparison {
  template<typename X, typename Y>
  auto operator()(const X& x, const Y& y) const noexcept -> bool {
    return rep_(x) < rep_(y);
  }

  private:
  static auto rep_(const timing_impl::duration& d) noexcept -> duration::rep {
    return d.count();
  }

  static auto rep_(const timing_impl::bucket& b) noexcept -> duration::rep {
    return b.le;
  }
};


timing_impl::timing_impl(const std::vector<duration>& thresholds)
: v_(thresholds.size())
{
  auto v_iter = v_.begin();
  for (const auto& threshold : thresholds) {
    if (v_iter != v_.begin()) {
      if (threshold.count() < std::prev(v_iter)->le)
        throw std::logic_error("unsorted thresholds for timing metric");
      if (threshold.count() == std::prev(v_iter)->le)
        throw std::logic_error("duplicate thresholds for timing metric");
    }

    v_iter++->le = threshold.count();
  }
}

void timing_impl::inc(duration d, std::uint64_t v) noexcept {
  const auto iter = std::lower_bound(
      v_.begin(), v_.end(),
      d,
      search_comparison());
  if (iter == v_.end())
    large_v_.fetch_add(v, std::memory_order_relaxed);
  else
    iter->v.fetch_add(v, std::memory_order_relaxed);
}

auto timing_impl::get_histogram() const -> std::tuple<std::vector<histogram_entry>, std::uint64_t> {
  std::vector<histogram_entry> h;
  if (!v_.empty()) {
    h.reserve(v_.size());
    for (const auto& bucket : v_) {
      h.push_back(histogram_entry{
          duration(bucket.le),
          bucket.v.load(std::memory_order_relaxed)
      });
    }
  }

  return std::make_tuple(std::move(h), large_v_.load(std::memory_order_relaxed));
}

auto timing_impl::default_buckets() -> std::vector<duration> {
  return std::vector<duration>({
      std::chrono::milliseconds(1),
      std::chrono::milliseconds(2),
      std::chrono::milliseconds(3),
      std::chrono::milliseconds(4),
      std::chrono::milliseconds(5),
      std::chrono::milliseconds(6),
      std::chrono::milliseconds(7),
      std::chrono::milliseconds(8),
      std::chrono::milliseconds(9),
      std::chrono::milliseconds(10),
      std::chrono::milliseconds(20),
      std::chrono::milliseconds(30),
      std::chrono::milliseconds(40),
      std::chrono::milliseconds(50),
      std::chrono::milliseconds(60),
      std::chrono::milliseconds(70),
      std::chrono::milliseconds(80),
      std::chrono::milliseconds(90),
      std::chrono::milliseconds(100),
      std::chrono::milliseconds(200),
      std::chrono::milliseconds(300),
      std::chrono::milliseconds(400),
      std::chrono::milliseconds(500),
      std::chrono::milliseconds(600),
      std::chrono::milliseconds(700),
      std::chrono::milliseconds(800),
      std::chrono::milliseconds(900),
      std::chrono::seconds(1),
      std::chrono::seconds(2),
      std::chrono::seconds(3),
      std::chrono::seconds(4),
      std::chrono::seconds(5),
      std::chrono::seconds(6),
      std::chrono::seconds(7),
      std::chrono::seconds(8),
      std::chrono::seconds(9),
      std::chrono::seconds(10),
      std::chrono::seconds(20),
      std::chrono::seconds(30),
      std::chrono::seconds(40),
      std::chrono::seconds(50),
      std::chrono::seconds(60),
      std::chrono::seconds(70),
      std::chrono::seconds(80),
      std::chrono::seconds(90),
      std::chrono::seconds(100)
  });
}


} /* namespace instrumentation::detail */

namespace instrumentation {


template class timing_vector<>;


} /* namespace instrumentation */
