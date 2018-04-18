#ifndef INSTRUMENTATION_BASIC_METRIC_H
#define INSTRUMENTATION_BASIC_METRIC_H

#include <instrumentation/fwd.h>
#include <instrumentation/visitor.h>
#include <instrumentation/tags.h>
#include <instrumentation/instrumentation_export_.h>
#include <atomic>
#include <vector>
#include <string_view>

namespace instrumentation {


class instrumentation_export_ basic_metric {
  friend class instrumentation::group;

 public:
  basic_metric(std::string_view local_name, group& parent, const tag_map& tags);
  template<std::size_t N>
  basic_metric(std::string_view local_name, group& parent, tags_<N> tags);
  basic_metric(const basic_metric&) = delete;
  virtual ~basic_metric() noexcept;

  virtual auto visit(visitor& v) const -> void = 0;

  const std::vector<std::string_view> name;
  const tag_map tags;

 protected:
  // Must be called by derived constructor.
  auto enable() noexcept -> void;
  // Must be called by derived destructor.
  auto disable() noexcept -> void;

 private:
  instrumentation_local_
  auto visit_before_destroy_() noexcept -> void;

  group& parent_;
  basic_metric* sibling_ = nullptr;
  bool enabled_ = false;
};


template<std::size_t N>
basic_metric::basic_metric(std::string_view local_name, group& parent, tags_<N> tags)
: basic_metric(local_name, parent, tags.as_map())
{}


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_BASIC_METRIC_H */
