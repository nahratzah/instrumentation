#include <instrumentation/visitor.h>
#include <instrumentation/group.h>
#include <algorithm>

namespace instrumentation {


visitor::~visitor() noexcept {}

auto visitor::operator()(const group& g) -> void {
  std::for_each(
      g.begin(), g.end(),
      [this](const hierarchy& h) { h.visit(*this); });
}


} /* namespace instrumentation */
