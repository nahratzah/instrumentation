#ifndef INSTRUMENTATION_VISITOR_H
#define INSTRUMENTATION_VISITOR_H

#include <cstdint>
#include <string>
#include <instrumentation/fwd.h>
#include <instrumentation/instrumentation_export_.h>

namespace instrumentation {


class instrumentation_export_ visitor {
 public:
  virtual ~visitor() noexcept;

  virtual auto operator()(const group& g) -> void;
  virtual auto operator()(const counter& c) -> void = 0;
  virtual auto operator()(const gauge<bool>& g) -> void = 0;
  virtual auto operator()(const gauge<std::int64_t>& g) -> void = 0;
  virtual auto operator()(const gauge<double>& g) -> void = 0;
  virtual auto operator()(const gauge<std::string>& g) -> void = 0;
};


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_VISITOR_H */
