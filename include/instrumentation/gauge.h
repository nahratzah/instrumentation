#ifndef INSTRUMENTATION_GAUGE_H
#define INSTRUMENTATION_GAUGE_H

#include <string_view>
#include <functional>
#include <instrumentation/hierarchy.h>
#include <instrumentation/tags.h>
#include <instrumentation/visitor.h>
#include <instrumentation/instrumentation_export_.h>

namespace instrumentation {


template<typename> class gauge;

template<>
class instrumentation_export_ gauge<bool> final
: public hierarchy
{
 public:
  using fn_type = std::function<bool()>;

  gauge(std::string_view local_name, fn_type fn, class tags t = {}) noexcept
  : hierarchy(local_name, std::move(t)),
    fn_(std::move(fn))
  {
    this->enable();
  }

  gauge(std::string_view local_name, fn_type fn, group& parent, class tags t = {}) noexcept
  : hierarchy(local_name, parent, std::move(t)),
    fn_(std::move(fn))
  {
    this->enable();
  }

  ~gauge() noexcept override;

  auto visit(visitor& v) const -> void override;

  auto operator*() const
  -> bool {
    return fn_();
  }

 private:
  fn_type fn_;
};

template<>
class instrumentation_export_ gauge<std::int64_t> final
: public hierarchy
{
 public:
  using fn_type = std::function<std::int64_t()>;

  gauge(std::string_view local_name, fn_type fn, class tags t = {}) noexcept
  : hierarchy(local_name, std::move(t)),
    fn_(std::move(fn))
  {
    this->enable();
  }

  gauge(std::string_view local_name, fn_type fn, group& parent, class tags t = {}) noexcept
  : hierarchy(local_name, parent, std::move(t)),
    fn_(std::move(fn))
  {
    this->enable();
  }

  ~gauge() noexcept override;

  auto visit(visitor& v) const -> void override;

  auto operator*() const
  -> std::int64_t {
    return fn_();
  }

 private:
  fn_type fn_;
};

template<>
class instrumentation_export_ gauge<double> final
: public hierarchy
{
 public:
  using fn_type = std::function<double()>;

  gauge(std::string_view local_name, fn_type fn, class tags t = {}) noexcept
  : hierarchy(local_name, std::move(t)),
    fn_(std::move(fn))
  {
    this->enable();
  }

  gauge(std::string_view local_name, fn_type fn, group& parent, class tags t = {}) noexcept
  : hierarchy(local_name, parent, std::move(t)),
    fn_(std::move(fn))
  {
    this->enable();
  }

  ~gauge() noexcept override;

  auto visit(visitor& v) const -> void override;

  auto operator*() const
  -> double {
    return fn_();
  }

 private:
  fn_type fn_;
};

template<>
class instrumentation_export_ gauge<std::string> final
: public hierarchy
{
 public:
  using fn_type = std::function<std::string()>;

  gauge(std::string_view local_name, fn_type fn, class tags t = {}) noexcept
  : hierarchy(local_name, std::move(t)),
    fn_(std::move(fn))
  {
    this->enable();
  }

  gauge(std::string_view local_name, fn_type fn, group& parent, class tags t = {}) noexcept
  : hierarchy(local_name, parent, std::move(t)),
    fn_(std::move(fn))
  {
    this->enable();
  }

  ~gauge() noexcept override;

  auto visit(visitor& v) const -> void override;

  auto operator*() const
  -> std::string {
    return fn_();
  }

 private:
  fn_type fn_;
};


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_GAUGE_H */
