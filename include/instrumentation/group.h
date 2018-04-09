#ifndef INSTRUMENTATION_GROUP_H
#define INSTRUMENTATION_GROUP_H

#include <cassert>
#include <cstddef>
#include <mutex>
#include <instrumentation/hierarchy.h>
#include <instrumentation/instrumentation_export_.h>

namespace instrumentation {


class instrumentation_export_ group final
: public hierarchy
{
 public:
  class iterator;

  using hierarchy::hierarchy;
  ~group() noexcept override;

  auto add(hierarchy& g) noexcept -> void;
  auto erase(hierarchy& g) noexcept -> void;

  auto visit(visitor& v) const -> void override;

  auto begin() const noexcept -> iterator;
  constexpr auto end() const noexcept -> iterator;

 private:
  std::mutex mtx_;
  hierarchy* child_ = nullptr; // Protected by mtx_
};

class instrumentation_local_ group::iterator {
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
