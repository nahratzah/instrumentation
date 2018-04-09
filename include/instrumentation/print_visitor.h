#ifndef INSTRUMENTATION_PRINT_VISITOR_H
#define INSTRUMENTATION_PRINT_VISITOR_H

#include <instrumentation/instrumentation_export_.h>
#include <instrumentation/visitor.h>
#include <instrumentation/hierarchy.h>
#include <instrumentation/fwd.h>
#include <iosfwd>

namespace instrumentation {


class instrumentation_export_ print_visitor final
: public visitor
{
 public:
  print_visitor();

  print_visitor(std::ostream& out)
  : out_(out)
  {}

  ~print_visitor() noexcept;

  auto operator()(const counter& c) -> void override;
  auto operator()(const gauge<bool>& g) -> void override;
  auto operator()(const gauge<std::int64_t>& g) -> void override;
  auto operator()(const gauge<double>& g) -> void override;
  auto operator()(const gauge<std::string>& g) -> void override;
  auto operator()(const timing& t) -> void override;

 private:
  instrumentation_local_ auto print_name_(const hierarchy& h) -> void;
  instrumentation_local_ auto print_tags_(const hierarchy& h) -> void;

  std::ostream& out_;
};


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_PRINT_VISITOR_H */
