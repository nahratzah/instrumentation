#ifndef INSTRUMENTATION_PROMETHEUS_H
#define INSTRUMENTATION_PROMETHEUS_H

#include <instrumentation/detail/export_.h>
#include <instrumentation/fwd.h>
#include <iosfwd>
#include <string>

namespace instrumentation {


instrumentation_export_
void collect_prometheus(std::ostream& out);
instrumentation_export_
void collect_prometheus(std::ostream& out, const engine& e);

instrumentation_export_
auto collect_prometheus() -> std::string;
instrumentation_export_
auto collect_prometheus(const engine& e) -> std::string;


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_PROMETHEUS_H */
