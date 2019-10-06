#ifndef INSTRUMENTATION_GAUGE_H
#define INSTRUMENTATION_GAUGE_H

#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>
#include <instrumentation/tags.h>

namespace instrumentation {


class gauge_intf {
  protected:
  virtual ~gauge_intf() noexcept;

  public:
  void inc() noexcept;
  void inc(double d) noexcept;
  void dec() noexcept;
  void dec(double d) noexcept;
  void set(double d) noexcept;

  private:
  virtual void do_inc(double d) noexcept = 0;
  virtual void do_set(double d) noexcept = 0;
};


class gauge {
  public:
  gauge() = default;

  explicit gauge(std::string_view name);
  gauge(std::string_view name, std::initializer_list<std::pair<const std::string, tags::tag_value>> tags);
  gauge(std::string_view name, instrumentation::tags tags);

  gauge(std::shared_ptr<gauge_intf> impl) noexcept
  : impl_(std::move(impl))
  {}

  void operator++() const noexcept;
  void operator++(int) const noexcept;
  void operator--() const noexcept;
  void operator--(int) const noexcept;
  void operator+=(double d) const noexcept;
  void operator-=(double d) const noexcept;
  void operator=(double d) const noexcept;

  private:
  std::shared_ptr<gauge_intf> impl_;
};


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_GAUGE_H */
