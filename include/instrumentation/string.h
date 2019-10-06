#ifndef INSTRUMENTATION_STRING_H
#define INSTRUMENTATION_STRING_H

#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>
#include <instrumentation/tags.h>

namespace instrumentation {


class string_intf {
  protected:
  virtual ~string_intf() noexcept;

  public:
  void set(std::string_view s) noexcept;

  private:
  virtual void do_set(std::string_view s) noexcept = 0;
};


class string {
  public:
  string() = default;

  explicit string(std::string_view name);
  string(std::string_view name, std::initializer_list<std::pair<const std::string, tags::tag_value>> tags);
  string(std::string_view name, instrumentation::tags tags);

  string(std::shared_ptr<string_intf> impl) noexcept
  : impl_(std::move(impl))
  {}

  void operator=(std::string_view s) const noexcept;

  private:
  std::shared_ptr<string_intf> impl_;
};


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_STRING_H */
