#ifndef INSTRUMENTATION_GAUGE_H
#define INSTRUMENTATION_GAUGE_H

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <functional>
#include <instrumentation/basic_metric.h>
#include <instrumentation/tags.h>
#include <instrumentation/visitor.h>
#include <instrumentation/instrumentation_export_.h>

namespace instrumentation {


template<typename> class gauge;

template<>
class instrumentation_export_ gauge<bool> final
: public basic_metric
{
 public:
  using fn_type = std::function<bool()>;

  gauge(std::string_view local_name, fn_type fn, group& parent, const tag_map& t = {}) noexcept
  : basic_metric(local_name, parent, t),
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
: public basic_metric
{
 public:
  using fn_type = std::function<std::int64_t()>;

  gauge(std::string_view local_name, fn_type fn, group& parent, const tag_map& t = {}) noexcept
  : basic_metric(local_name, parent, std::move(t)),
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
: public basic_metric
{
 public:
  using fn_type = std::function<double()>;

  template<std::size_t N>
  gauge(std::string_view local_name, fn_type fn, group& parent, const tag_map& t = {}) noexcept
  : basic_metric(local_name, parent, std::move(t)),
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
: public basic_metric
{
 public:
  using fn_type = std::function<std::string()>;

  template<std::size_t N>
  gauge(std::string_view local_name, fn_type fn, group& parent, const tag_map& t = {}) noexcept
  : basic_metric(local_name, parent, std::move(t)),
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
