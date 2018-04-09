#include <instrumentation/visitor.h>

namespace instrumentation {


visitor::~visitor() noexcept {}

auto visitor::operator()(const group& g) -> void {}


} /* namespace instrumentation */
