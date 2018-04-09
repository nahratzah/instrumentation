#ifndef INSTRUMENTATION_HIERARCHY_H
#define INSTRUMENTATION_HIERARCHY_H

#include <instrumentation/fwd.h>
#include <instrumentation/visitor.h>
#include <instrumentation/instrumentation_export_.h>

namespace instrumentation {


class instrumentation_export_ hierarchy {
  friend class instrumentation::group;

 public:
  constexpr hierarchy() noexcept = default;
  hierarchy(const hierarchy&) = delete;
  virtual ~hierarchy() noexcept;

  virtual auto visit(visitor& v) const -> void = 0;

 private:
  hierarchy* sibling_ = nullptr;
};


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_HIERARCHY_H */
