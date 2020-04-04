#ifndef INSTRUMENTATION_PROMETHEUS_ENGINE_H
#define INSTRUMENTATION_PROMETHEUS_ENGINE_H

#include <instrumentation/fwd.h>
#include <iosfwd>
#include <string>

namespace instrumentation {


void collect_prometheus(std::ostream& out);
void collect_prometheus(std::ostream& out, const engine& e);

auto collect_prometheus() -> std::string;
auto collect_prometheus(const engine& e) -> std::string;


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_PROMETHEUS_ENGINE_H */
