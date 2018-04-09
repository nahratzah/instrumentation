#include <instrumentation/hierarchy.h>
#include <instrumentation/group.h>
#include <sstream>
#include <thread>

namespace instrumentation {


hierarchy::~hierarchy() noexcept {}

auto hierarchy::name() const
-> std::vector<std::string_view> {
  using vector = std::vector<std::string_view>;

  vector result = (parent_ == nullptr ? vector() : parent_->name());
  result.push_back(local_name);
  return result;
}

auto hierarchy::tags() const
-> tags::map_type {
  tags::map_type result = *local_tags;
  bool add_tls_entry = local_tags.tls_entry();

  for (const hierarchy* g = parent_;
      g != nullptr;
      g = g->parent_) {
    add_tls_entry |= g->local_tags.tls_entry();
    std::copy(
        g->local_tags->begin(), g->local_tags->end(),
        std::inserter(result, result.end()));
  }

  if (add_tls_entry) {
    result[tags::tls_entry_key] =
        (std::ostringstream() << std::this_thread::get_id()).str();
  }

  return result;
}

auto hierarchy::do_enable_() noexcept -> void {
  assert(parent_ != nullptr);
  parent_->add(*this);
}

auto hierarchy::disable() noexcept -> void {
  visit_before_destroy_(*this);
  if (enabled_.load(std::memory_order_acquire)) {
    assert(parent_ != nullptr);
    parent_->erase(*this);
  }
}


} /* namespace instrumentation */
