#ifndef INSTRUMENTATION_COUNTER_H
#define INSTRUMENTATION_COUNTER_H

#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>
#include <instrumentation/tags.h>

namespace instrumentation {


class counter_intf {
  protected:
  virtual ~counter_intf() noexcept;

  public:
  void inc() noexcept;
  void inc(double d) noexcept;

  private:
  virtual void do_inc(double d) noexcept = 0;
};


class counter {
  public:
  counter() = default;

  explicit counter(std::string_view name);
  counter(std::string_view name, std::initializer_list<std::pair<const std::string, tags::tag_value>> tags);
  counter(std::string_view name, instrumentation::tags tags);

  counter(std::shared_ptr<counter_intf> impl) noexcept
  : impl_(std::move(impl))
  {}

  void operator++() const noexcept;
  void operator++(int) const noexcept;
  void operator+=(double d) const noexcept;

  private:
  std::shared_ptr<counter_intf> impl_;
};


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_COUNTER_H */
