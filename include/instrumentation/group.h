#ifndef INSTRUMENTATION_GROUP_H
#define INSTRUMENTATION_GROUP_H

#include <atomic>
#include <cassert>
#include <cstddef>
#include <mutex>
#include <string_view>
#include <vector>
#include <instrumentation/hierarchy.h>
#include <instrumentation/tags.h>
#include <instrumentation/instrumentation_export_.h>

namespace instrumentation {


class instrumentation_export_ group final
: public hierarchy
{
 public:
  class instrumentation_local_ iterator;

  group(std::string_view name, tags t = {}) noexcept;
  group(std::string_view name, group& parent, tags t = {}) noexcept;
  group(const group&) = delete;
  ~group() noexcept override;

  auto name() const -> std::vector<std::string_view>;
  auto tags() const -> tags::map_type;

  auto enable()
  noexcept
  -> void {
    bool expect_false = false;
    if (parent_ != nullptr &&
        enabled_.compare_exchange_strong(
            expect_false, true,
            std::memory_order_acquire, std::memory_order_relaxed)) {
      parent_->add(*this);
    }
  }

  auto add(hierarchy& g) noexcept -> void;
  auto erase(hierarchy& g) noexcept -> void;

  auto visit(visitor& v) const -> void override;

  const std::string_view local_name;
  const class tags local_tags;

  auto begin() const noexcept -> iterator;
  constexpr auto end() const noexcept -> iterator;

 private:
  std::atomic<bool> enabled_{ false };
  group* parent_ = nullptr;
  std::mutex mtx_;
  hierarchy* child_ = nullptr; // Protected by mtx_
};

class group::iterator {
 public:
  using difference_type = std::ptrdiff_t;
  using value_type = hierarchy;
  using pointer = const hierarchy*;
  using reference = const hierarchy&;
  using iterator_category = std::forward_iterator_tag;

  explicit constexpr iterator(const hierarchy* child) noexcept
  : child_(child)
  {}

  auto operator++()
  -> iterator& {
    assert(child_ != nullptr);
    child_ = child_->sibling_;
    return *this;
  }

  auto operator++(int)
  -> iterator {
    assert(child_ != nullptr);
    return iterator(std::exchange(child_, child_->sibling_));
  }

  auto operator==(const iterator& y) const noexcept
  -> bool {
    return child_ == y.child_;
  }

  auto operator!=(const iterator& y) const noexcept
  -> bool {
    return !(*this == y);
  }

  auto operator*() const -> reference {
    assert(child_ != nullptr);
    return *child_;
  }

  auto operator->() const -> pointer {
    assert(child_ != nullptr);
    return child_;
  }

 private:
  const hierarchy* child_;
};

inline auto group::begin() const noexcept -> iterator {
  return iterator(child_);
}

constexpr auto group::end() const noexcept -> iterator {
  return iterator(nullptr);
}


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_GROUP_H */
