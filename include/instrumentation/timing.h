#ifndef INSTRUMENTATION_TIMING_H
#define INSTRUMENTATION_TIMING_H

#include <instrumentation/instrumentation_export_.h>
#include <instrumentation/hierarchy.h>
#include <chrono>
#include <cstddef>
#include <atomic>

namespace instrumentation {


class instrumentation_export_ timing final
: public hierarchy
{
 private:
  using atom_vector = std::vector<std::atomic<std::uint64_t>>;

 public:
  using clock_type = std::chrono::high_resolution_clock;
  using duration = clock_type::duration;
  static constexpr std::chrono::milliseconds dfl_resolution{1};
  static constexpr std::size_t dfl_buckets = std::chrono::seconds{1} / dfl_resolution;

  class iterator;

  struct bucket {
    duration lo, hi;
    std::uint64_t count;
  };

  static constexpr auto max()
  noexcept
  -> duration {
    return duration::max();
  }

  timing(std::string_view local_name, duration resolution = dfl_resolution, std::size_t buckets = dfl_buckets, class tags t = {}) noexcept;
  timing(std::string_view local_name, duration resolution, std::size_t buckets, group& parent, class tags t = {}) noexcept;

  timing(std::string_view local_name, group& parent, class tags t = {}) noexcept
  : timing(local_name, dfl_resolution, dfl_buckets, parent, std::move(t))
  {}

  ~timing() noexcept override;

  auto visit(visitor& v) const -> void override;

  auto add(duration d) noexcept -> void;

  auto begin() const noexcept -> iterator;
  auto end() const noexcept -> iterator;

 private:
  atom_vector timings_;
  duration resolution_;
};

class instrumentation_local_ timing::iterator {
 public:
  using difference_type = std::ptrdiff_t;
  using value_type = timing::bucket;
  using reference = timing::bucket;
  using pointer = void;
  using iterator_category = std::input_iterator_tag;

  iterator(duration lo, duration resolution, atom_vector::const_iterator iter) noexcept
  : lo_(lo),
    resolution_(resolution),
    iter_(iter)
  {}

  auto operator++()
  -> iterator& {
    lo_ += resolution_;
    ++iter_;
    return *this;
  }

  auto operator++(int)
  -> iterator {
    iterator copy = *this;
    ++*this;
    return copy;
  }

  auto operator==(const iterator& y) const
  noexcept
  -> bool {
    return iter_ == y.iter_;
  }

  auto operator!=(const iterator& y) const
  noexcept
  -> bool {
    return !(*this == y);
  }

  auto operator*() const
  -> reference {
    return bucket{ lo_, lo_ + resolution_, iter_->load(std::memory_order_relaxed) };
  }

 private:
  duration lo_;
  duration resolution_;
  atom_vector::const_iterator iter_;
};

inline auto timing::begin() const noexcept -> iterator {
  return iterator(duration(0), resolution_, timings_.begin());
}

inline auto timing::end() const noexcept -> iterator {
  return iterator(timings_.size() * resolution_, resolution_, timings_.end());
}


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_TIMING_H */
