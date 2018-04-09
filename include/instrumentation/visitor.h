#ifndef INSTRUMENTATION_VISITOR_H
#define INSTRUMENTATION_VISITOR_H

#include <instrumentation/fwd.h>
#include <instrumentation/instrumentation_export_.h>

namespace instrumentation {


class instrumentation_export_ visitor {
 public:
  virtual ~visitor() noexcept;

  virtual auto operator()(const group& g) -> void;
  virtual auto operator()(const counter& c) -> void = 0;
};


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_VISITOR_H */
