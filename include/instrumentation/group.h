#ifndef INSTRUMENTATION_GROUP_H
#define INSTRUMENTATION_GROUP_H

#include <algorithm>
#include <atomic>
#include <mutex>
#include <string_view>
#include <utility>
#include <vector>
#include <instrumentation/hierarchy.h>
#include <instrumentation/tags.h>

namespace instrumentation {


class group final
: public hierarchy
{
 public:
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

 private:
  std::atomic<bool> enabled_{ false };
  group* parent_ = nullptr;
  std::mutex mtx_;
  hierarchy* child_ = nullptr; // Protected by mtx_
};


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_GROUP_H */
