#ifndef INSTRUMENTATION_HIERARCHY_H
#define INSTRUMENTATION_HIERARCHY_H

#include <instrumentation/fwd.h>
#include <instrumentation/visitor.h>
#include <instrumentation/tags.h>
#include <instrumentation/instrumentation_export_.h>

namespace instrumentation {


class instrumentation_export_ hierarchy {
  friend class instrumentation::group;

 public:
  hierarchy(std::string_view local_name, class tags t = {}) noexcept
  : local_name(local_name),
    local_tags(std::move(t))
  {}

  hierarchy(std::string_view local_name, group& parent, class tags t = {}) noexcept
  : local_name(local_name),
    local_tags(std::move(t)),
    parent_(&parent)
  {}

  hierarchy(const hierarchy&) = delete;
  virtual ~hierarchy() noexcept;

  virtual auto visit(visitor& v) const -> void = 0;

  auto enable()
  noexcept
  -> void {
    bool expect_false = false;
    if (parent_ != nullptr &&
        enabled_.compare_exchange_strong(
            expect_false, true,
            std::memory_order_acquire, std::memory_order_relaxed)) {
      do_enable_();
    }
  }

  auto name() const -> std::vector<std::string_view>;
  auto tags() const -> tags::map_type;

  const std::string_view local_name;
  const class tags local_tags;

 protected:
  auto disable() noexcept -> void;

 private:
  auto do_enable_() noexcept -> void;

  std::atomic<bool> enabled_{ false };
  group* parent_ = nullptr;
  hierarchy* sibling_ = nullptr;
};


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_HIERARCHY_H */
