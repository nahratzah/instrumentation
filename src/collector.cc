#include <instrumentation/collector.h>

namespace instrumentation {


collector::~collector() noexcept = default;

void collector::visit_description(const metric_name& name, std::string_view description) {}


} /* namespace instrumentation */
