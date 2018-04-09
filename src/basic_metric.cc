#include <instrumentation/basic_metric.h>
#include <algorithm>
#include <utility>

namespace instrumentation {


basic_metric::basic_metric(std::string_view name, class tags t) noexcept
: local_name(name),
  local_tags(std::move(t))
{}

basic_metric::basic_metric(std::string_view name, group& parent, class tags t) noexcept
: local_name(name),
  local_tags(std::move(t)),
  parent_(&parent)
{}

basic_metric::~basic_metric() noexcept {
  if (parent_ != nullptr && enabled_.load(std::memory_order_acquire))
    parent_->erase(*this);
}

auto basic_metric::name() const
-> std::vector<std::string_view> {
  using vector = std::vector<std::string_view>;

  vector result = (parent_ == nullptr ? vector() : parent_->name());
  result.push_back(local_name);
  return result;
}

auto basic_metric::tags() const
-> tags::map_type {
  tags::map_type result = (parent_ == nullptr
      ? tags::map_type()
      : parent_->tags());
  std::for_each(
      local_tags->begin(), local_tags->end(),
      [&result](const auto& x) { result[x.first] = x.second; });
  return result;
}


} /* namespace instrumentation */
