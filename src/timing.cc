#include <instrumentation/timing.h>
#include <instrumentation/engine.h>

namespace instrumentation {


timing_intf::~timing_intf() noexcept = default;

void timing_intf::add(duration d) noexcept {
  do_add(d);
}


timing::timing(std::string_view name, duration resolution, std::size_t buckets)
: timing(engine::global().new_timing(name, resolution, buckets))
{}

timing::timing(std::string_view name, std::initializer_list<std::pair<const std::string, tags::tag_value>> tags, duration resolution, std::size_t buckets)
: timing(name, instrumentation::tags(tags), resolution, buckets)
{}

timing::timing(std::string_view name, instrumentation::tags tags, duration resolution, std::size_t buckets)
: timing(engine::global().new_timing(name, std::move(tags), resolution, buckets))
{}

auto timing::cumulative(std::string_view name, std::initializer_list<std::pair<const std::string, tags::tag_value>> tags) -> timing {
  return cumulative(name, instrumentation::tags(tags));
}

auto timing::cumulative(std::string_view name, instrumentation::tags tags) -> timing {
  return engine::global().new_cumulative_timing(name, std::move(tags));
}

void timing::operator<<(duration d) const noexcept {
  if (impl_) impl_->add(d);
}


} /* namespace instrumentation */
