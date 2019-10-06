#include <instrumentation/gauge.h>
#include <instrumentation/engine.h>

namespace instrumentation {


gauge_intf::~gauge_intf() noexcept = default;

void gauge_intf::inc() noexcept {
  do_inc(1);
}

void gauge_intf::inc(double d) noexcept {
  do_inc(d);
}

void gauge_intf::dec() noexcept {
  do_inc(-1);
}

void gauge_intf::dec(double d) noexcept {
  do_inc(-d);
}

void gauge_intf::set(double d) noexcept {
  do_set(d);
}


gauge::gauge(std::string_view name)
: gauge(engine::global().new_gauge(name))
{}

gauge::gauge(std::string_view name, std::initializer_list<std::pair<const std::string, tags::tag_value>> tags)
: gauge(name, instrumentation::tags(tags))
{}

gauge::gauge(std::string_view name, instrumentation::tags tags)
: gauge(engine::global().new_gauge(name, std::move(tags)))
{}

void gauge::operator++() const noexcept {
  if (impl_) impl_->inc();
}

void gauge::operator++(int) const noexcept {
  if (impl_) impl_->inc();
}

void gauge::operator--() const noexcept {
  if (impl_) impl_->dec();
}

void gauge::operator--(int) const noexcept {
  if (impl_) impl_->dec();
}

void gauge::operator+=(double d) const noexcept {
  if (impl_) impl_->inc(d);
}

void gauge::operator-=(double d) const noexcept {
  if (impl_) impl_->dec(d);
}

void gauge::operator=(double d) const noexcept {
  if (impl_) impl_->set(d);
}


} /* namespace instrumentation */
