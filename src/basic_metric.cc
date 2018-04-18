#include <instrumentation/basic_metric.h>
#include <instrumentation/group.h>
#include <sstream>
#include <thread>

namespace instrumentation {
namespace {


auto create_metric_name_(std::string_view local_name, group& parent)
-> std::vector<std::string_view> {
  std::vector<std::string_view> result = parent.name();
  result.push_back(local_name);
  return result;
}

auto create_metric_tags_(group& parent, const tag_map& tags)
-> tag_map {
  tag_map result = parent.tags();
  std::for_each(
      tags.begin(), tags.end(),
      [&result](const auto& pair) { result[pair.first] = pair.second; });
  return result;
}


} /* namespace instrumentation::<unnamed> */


basic_metric::basic_metric(std::string_view local_name, group& parent, const tag_map& tags)
: name(create_metric_name_(local_name, parent)),
  tags(create_metric_tags_(parent, tags)),
  parent_(parent)
{}

basic_metric::~basic_metric() noexcept {
  assert(!enabled_);
}

auto basic_metric::enable() noexcept -> void {
  assert(!enabled_);

  parent_.add_(*this);
  enabled_ = true;
}

auto basic_metric::disable() noexcept -> void {
  assert(enabled_);

  visit_before_destroy_();
  parent_.erase_(*this);
  enabled_ = false;
}


} /* namespace instrumentation */
