#include <instrumentation/counter.h>

namespace instrumentation {


counter::~counter() noexcept {
  disable();
}

auto counter::visit(visitor& v) const
-> void {
  v(*this);
}


} /* namespace instrumentation */
