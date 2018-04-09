#include <instrumentation/group.h>

namespace instrumentation {


group::group(std::string_view name, class tags t) noexcept
: local_name(name),
  local_tags(std::move(t))
{}

group::group(std::string_view name, group& parent, class tags t) noexcept
: local_name(name),
  local_tags(std::move(t)),
  parent_(&parent)
{}

group::~group() noexcept {
  if (parent_ != nullptr && enabled_.load(std::memory_order_acquire))
    parent_->erase(*this);
}

auto group::name() const
-> std::vector<std::string_view> {
  using vector = std::vector<std::string_view>;

  vector result = (parent_ == nullptr ? vector() : parent_->name());
  result.push_back(local_name);
  return result;
}

auto group::tags() const
-> tags::map_type {
  tags::map_type result = *local_tags;
  for (auto g = parent_;
      g != nullptr;
      g = g->parent_) {
    std::copy(
        g->local_tags->begin(), g->local_tags->end(),
        std::inserter(result, result.end()));
  }
  return result;
}

auto group::add(hierarchy& g)
noexcept
-> void {
  enable();
  std::lock_guard<std::mutex> lck{ mtx_ };
  g.sibling_ = std::exchange(child_, &g);
}

auto group::erase(hierarchy& g)
noexcept
-> void {
  std::lock_guard<std::mutex> lck{ mtx_ };
  for (hierarchy** child_iter = &child_;
      *child_iter != nullptr;
      child_iter = &(*child_iter)->sibling_) {
    if (*child_iter == &g) {
      *child_iter = std::exchange(g.sibling_, nullptr);
      return;
    }
  }
}

auto group::visit(visitor& v) const
-> void {
  v(*this);
  for (hierarchy* child_iter = child_;
      child_iter != nullptr;
      child_iter = child_iter->sibling_)
    child_iter->visit(v);
}


} /* namespace instrumentation */
