#ifndef INSTRUMENTATION_COLLECTOR_H
#define INSTRUMENTATION_COLLECTOR_H

#include <instrumentation/fwd.h>
#include <instrumentation/metric_name.h>
#include <instrumentation/tags.h>
#include <string_view>

namespace instrumentation {


class collector {
  public:
  virtual ~collector() noexcept = default;

  virtual void visit_description(const metric_name& name, std::string_view description);
  virtual void visit(const metric_name& name, const tags& tags, const counter& v) = 0;
};


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_COLLECTOR_H */
