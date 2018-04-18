#ifndef INSTRUMENTATION_GROUP_H
#define INSTRUMENTATION_GROUP_H

#include <cassert>
#include <cstddef>
#include <mutex>
#include <string_view>
#include <vector>
#include <instrumentation/tags.h>
#include <instrumentation/visitor.h>
#include <instrumentation/fwd.h>
#include <instrumentation/basic_metric.h>
#include <instrumentation/instrumentation_export_.h>

namespace instrumentation {


class instrumentation_export_ group {
 public:
  class subgroup_set;
  class submetric_set;
  friend class instrumentation::basic_metric;

  explicit constexpr group(const std::string_view& local_name)
  : local_name(local_name),
    parent_(nullptr)
  {}

  explicit constexpr group(const std::string_view& local_name, group& parent)
  : local_name(local_name),
    parent_(&parent)
  {}

 protected:
  explicit constexpr group(const group& y) noexcept
  : local_name(y.local_name),
    parent_(y.parent_)
  {}

 public:
  auto visit(visitor& v) const -> void;
  static auto mtx() noexcept -> std::recursive_mutex&;
  constexpr auto childgroups() const noexcept -> subgroup_set;
  constexpr auto childmetrics() const noexcept -> submetric_set;

  auto name() const -> std::vector<std::string_view>;
  auto tags() const -> tag_map;
  auto tags(tag_map& tag_set) const -> tag_map&;

  const std::string_view local_name;

 private:
  instrumentation_local_
  auto add_(group& g, std::unique_lock<std::recursive_mutex>& lck) noexcept -> void;
  instrumentation_local_
  auto erase_(group& g, std::unique_lock<std::recursive_mutex>& lck) noexcept -> void;
  instrumentation_local_
  auto add_(basic_metric& g) noexcept -> void;
  instrumentation_local_
  auto erase_(basic_metric& g) noexcept -> void;
  instrumentation_local_
  auto maybe_enable_(std::unique_lock<std::recursive_mutex>& lck) noexcept -> void;
  instrumentation_local_
  auto maybe_disable_(std::unique_lock<std::recursive_mutex>& lck) noexcept -> void;
  virtual auto apply_local_tags_(tag_map& map) const -> void = 0;

  group*const parent_;
  group* child_groups_ = nullptr;
  basic_metric* child_metrics_ = nullptr;
  group* sibling_ = nullptr;
  bool enabled_ = false;
};

template<std::size_t N>
class tagged_group
: public group
{
  template<std::size_t> friend class tagged_group;

 public:
  template<std::size_t NN = N, typename = std::enable_if_t<(NN == 0)>>
  explicit constexpr tagged_group(std::string_view local_name) noexcept
  : group(local_name)
  {}

  template<std::size_t NN = N, typename = std::enable_if_t<(NN == 0)>>
  explicit constexpr tagged_group(std::string_view local_name, group& parent) noexcept
  : group(local_name, parent)
  {}

 private:
  template<std::size_t NN = N, typename = std::enable_if_t<(NN > 0)>>
  constexpr tagged_group(tagged_group<N - 1>&& base, const tag_entry& entry)
  : group(std::move(base)),
    tag_set_(base.tag_set_ | entry)
  {}

 public:
  constexpr auto operator[](const tag_entry& entry) &&
  -> tagged_group<N + 1> {
    return tagged_group<N + 1>(std::move(*this), entry);
  }

 private:
  auto apply_local_tags_(tag_map& map) const -> void /*override*/ {
    tag_set_.apply(map);
  }

  tags_<N> tag_set_;
};

constexpr auto make_group(std::string_view local_name)
-> tagged_group<0> {
  return tagged_group<0>{ local_name };
}

constexpr auto make_group(std::string_view local_name, group& parent)
-> tagged_group<0> {
  return tagged_group<0>{ local_name, parent };
}

struct group::subgroup_set {
 public:
  class iterator;

  auto begin() const noexcept -> iterator;
  constexpr auto end() const noexcept -> iterator;

  const group* self_ = nullptr;
};

struct group::submetric_set {
 public:
  class iterator;

  auto begin() const noexcept -> iterator;
  constexpr auto end() const noexcept -> iterator;

  const group* self_ = nullptr;
};

class instrumentation_local_ group::subgroup_set::iterator {
 public:
  using difference_type = std::ptrdiff_t;
  using value_type = group;
  using pointer = const group*;
  using reference = const group&;
  using iterator_category = std::forward_iterator_tag;

  explicit constexpr iterator(const group* child) noexcept
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
  const group* child_;
};

class instrumentation_local_ group::submetric_set::iterator {
 public:
  using difference_type = std::ptrdiff_t;
  using value_type = basic_metric;
  using pointer = const basic_metric*;
  using reference = const basic_metric&;
  using iterator_category = std::forward_iterator_tag;

  explicit constexpr iterator(const basic_metric* child) noexcept
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
  const basic_metric* child_;
};

inline auto group::subgroup_set::begin() const noexcept -> iterator {
  return iterator(self_->child_groups_);
}

constexpr auto group::subgroup_set::end() const noexcept -> iterator {
  return iterator(nullptr);
}

inline auto group::submetric_set::begin() const noexcept -> iterator {
  return iterator(self_->child_metrics_);
}

constexpr auto group::submetric_set::end() const noexcept -> iterator {
  return iterator(nullptr);
}

constexpr auto group::childgroups() const
noexcept
-> subgroup_set {
  return subgroup_set{ this };
}

constexpr auto group::childmetrics() const
noexcept
-> submetric_set {
  return submetric_set{ this };
}


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_GROUP_H */
