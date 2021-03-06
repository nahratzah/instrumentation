#ifndef INSTRUMENTATION_COLLECTOR_H
#define INSTRUMENTATION_COLLECTOR_H

#include <instrumentation/detail/export_.h>
#include <instrumentation/fwd.h>
#include <instrumentation/metric_name.h>
#include <instrumentation/tags.h>
#include <string_view>

namespace instrumentation {


class instrumentation_export_ collector {
  public:
  virtual ~collector() noexcept;

  virtual void visit_description(const metric_name& name, std::string_view description);
  virtual void visit(const metric_name& name, const tags& tags, const counter& v) = 0;
  virtual void visit(const metric_name& name, const tags& tags, const gauge& v) = 0;
  virtual void visit(const metric_name& name, const tags& tags, const string& v) = 0;
  virtual void visit(const metric_name& name, const tags& tags, const timing& v) = 0;
};


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_COLLECTOR_H */
