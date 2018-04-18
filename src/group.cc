#include <instrumentation/group.h>
#include <mutex>
#include <algorithm>
#include <utility>
#include <cassert>

namespace instrumentation {


// Global mutex, used to synchronize registration/de-registration of sub groups.
auto group::mtx()
noexcept
-> std::recursive_mutex& {
  static std::recursive_mutex mtx;
  return mtx;
}

auto group::visit(visitor& v) const
-> void {
  v(*this);
}

auto group::name() const
-> std::vector<std::string_view> {
  // No locking: all member values that are used, are const.

  std::vector<std::string_view> result = (parent_ != nullptr
      ? parent_->name()
      : std::vector<std::string_view>());

  if (!local_name.empty())
    result.push_back(local_name);
  return result;
}

auto group::tags() const
-> tag_map {
  tag_map result;
  tags(result);
  return result;
}

auto group::tags(tag_map& tag_set) const
-> tag_map& {
  if (parent_ != nullptr)
    parent_->tags(tag_set);
  apply_local_tags_(tag_set);
  return tag_set;
}

auto group::add_(group& g, std::unique_lock<std::recursive_mutex>& lck)
noexcept
-> void {
  assert(lck.owns_lock() && lck.mutex() == &mtx());

  maybe_enable_(lck);
  g.sibling_ = std::exchange(child_groups_, &g);
}

auto group::erase_(group& g, std::unique_lock<std::recursive_mutex>& lck)
noexcept
-> void {
  assert(lck.owns_lock() && lck.mutex() == &mtx());

  for (group** child_iter = &child_groups_;
      *child_iter != nullptr;
      child_iter = &(*child_iter)->sibling_) {
    if (*child_iter == &g) {
      *child_iter = std::exchange(g.sibling_, nullptr);
      break;
    }
  }

  if (child_groups_ == nullptr)
    maybe_disable_(lck);
}

auto group::maybe_enable_(std::unique_lock<std::recursive_mutex>& lck)
noexcept
-> void {
  assert(lck.owns_lock() && lck.mutex() == &mtx());

  if (enabled_) return;
  if (parent_ == nullptr) return;

  parent_->add_(*this, lck);
  enabled_ = true;
}

auto group::maybe_disable_(std::unique_lock<std::recursive_mutex>& lck)
noexcept
-> void {
  assert(lck.owns_lock() && lck.mutex() == &mtx());

  if (!enabled_) return;

  if (parent_ != nullptr) parent_->erase_(*this, lck);
  enabled_ = false;
}

static_assert(std::is_trivially_destructible_v<group>);
static_assert(std::is_trivially_destructible_v<tagged_group<0>>);
static_assert(std::is_trivially_destructible_v<tagged_group<1>>);


} /* namespace instrumentation */
