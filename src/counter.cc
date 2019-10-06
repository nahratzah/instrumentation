#include <instrumentation/counter.h>
#include <instrumentation/engine.h>

namespace instrumentation {


counter_intf::~counter_intf() noexcept = default;

void counter_intf::inc() noexcept {
  do_inc(1);
}

void counter_intf::inc(double d) noexcept {
  if (d >= 0) do_inc(d);
}


counter::counter(std::string_view name)
: counter(engine::global().new_counter(name))
{}

counter::counter(std::string_view name, std::initializer_list<std::pair<const std::string, tags::tag_value>> tags)
: counter(name, instrumentation::tags(tags))
{}

counter::counter(std::string_view name, instrumentation::tags tags)
: counter(engine::global().new_counter(name, std::move(tags)))
{}

void counter::operator++() const noexcept {
  if (impl_) impl_->inc();
}

void counter::operator++(int) const noexcept {
  if (impl_) impl_->inc();
}

void counter::operator+=(double d) const noexcept {
  if (impl_) impl_->inc(d);
}


} /* namespace instrumentation */
