#ifndef INSTRUMENTATION_BASIC_METRIC_H
#define INSTRUMENTATION_BASIC_METRIC_H

#include <atomic>
#include <string_view>
#include <vector>
#include <instrumentation/hierarchy.h>
#include <instrumentation/tags.h>
#include <instrumentation/group.h>
#include <instrumentation/instrumentation_export_.h>

namespace instrumentation {


class instrumentation_export_ basic_metric
: public hierarchy
{
 public:
  basic_metric(std::string_view name, tags t = {}) noexcept;
  basic_metric(std::string_view name, group& parent, tags t = {}) noexcept;
  basic_metric(const basic_metric&) = delete;
  ~basic_metric() noexcept override;

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

  auto name() const -> std::vector<std::string_view>;
  auto tags() const -> tags::map_type;

  const std::string_view local_name;
  const class tags local_tags;

 private:
  std::atomic<bool> enabled_{ false };
  group* parent_ = nullptr;
};


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_BASIC_METRIC_H */
