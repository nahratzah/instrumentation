#include <instrumentation/visitor.h>
#include <instrumentation/group.h>
#include <algorithm>

namespace instrumentation {


visitor::~visitor() noexcept {}

auto visitor::operator()(const group& g) -> void {
  std::lock_guard<std::recursive_mutex> lck{ g.mtx() };
  for (const group& child : g.childgroups())
    child.visit(*this);
  for (const basic_metric& child : g.childmetrics())
    child.visit(*this);
}


} /* namespace instrumentation */
