#ifndef INSTRUMENTATION_GROUP_H
#define INSTRUMENTATION_GROUP_H

#include <mutex>
#include <string_view>

namespace instrumentation {


class group {
 public:
  constexpr group(std::string_view name) noexcept
  : group(name, nullptr)
  {}

  constexpr group(std::string_view name, group* parent) noexcept
  : name(name),
    parent_(nullptr)
  {}

  const std::string_view name;

  void enable() {
    std::call_once(registered_, &group::do_register_, this);
  }

 private:
  void do_register_() noexcept;

  group*const parent_;
  std::once_flag registered_;
};


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_GROUP_H */
