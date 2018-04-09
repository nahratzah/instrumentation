#include <instrumentation/timing.h>
#include <cassert>
#include <cstddef>

namespace instrumentation {


timing::timing(std::string_view local_name, duration resolution, std::size_t buckets, class tags t) noexcept
: hierarchy(local_name, std::move(t)),
  timings_(buckets + 1u),
  resolution_(resolution)
{}

timing::timing(std::string_view local_name, duration resolution, std::size_t buckets, group& parent, class tags t) noexcept
: hierarchy(local_name, parent, std::move(t)),
  timings_(buckets + 1u),
  resolution_(resolution)
{}

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
