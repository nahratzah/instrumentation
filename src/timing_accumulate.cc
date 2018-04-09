#include <instrumentation/timing_accumulate.h>

namespace instrumentation {


timing_accumulate::~timing_accumulate() noexcept {
  disable();
}

auto timing_accumulate::visit(visitor& v) const
-> void {
  v(*this);
}


} /* namespace instrumentation */
