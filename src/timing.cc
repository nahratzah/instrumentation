#include <instrumentation/timing.h>
#include <cassert>
#include <cstddef>

namespace instrumentation {


timing::~timing() noexcept {
  disable();
}

auto timing::visit(visitor& v) const
-> void {
  v(*this);
}

auto timing::add(duration d)
noexcept
-> void {
  assert(resolution_ > duration(0));

  std::size_t idx = d / resolution_;
  if (d < duration(0)) idx = 0;
  if (idx >= timings_.size()) idx = timings_.size() - 1u;

  timings_[idx].fetch_add(1u, std::memory_order_relaxed);
}


} /* namespace instrumentation */
