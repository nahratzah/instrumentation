#ifndef INSTRUMENTATION_VISITOR_H
#define INSTRUMENTATION_VISITOR_H

#include <instrumentation/fwd.h>

namespace instrumentation {


class visitor {
 public:
  virtual ~visitor() noexcept;

  virtual auto operator()(const group& g) -> void;
};


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_VISITOR_H */
