#include <instrumentation/gauge.h>

namespace instrumentation {


gauge<bool>::~gauge() noexcept {
  disable();
}

auto gauge<bool>::visit(visitor& v) const
-> void {
  v(*this);
}


gauge<std::int64_t>::~gauge() noexcept {
  disable();
}

auto gauge<std::int64_t>::visit(visitor& v) const
-> void {
  v(*this);
}


gauge<double>::~gauge() noexcept {
  disable();
}

auto gauge<double>::visit(visitor& v) const
-> void {
  v(*this);
}


gauge<std::string>::~gauge() noexcept {
  disable();
}

auto gauge<std::string>::visit(visitor& v) const
-> void {
  v(*this);
}


} /* namespace instrumentation */
