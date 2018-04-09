#include <instrumentation/print_visitor.h>
#include <instrumentation/counter.h>
#include <instrumentation/gauge.h>
#include <instrumentation/timing.h>
#include <iostream>
#include <sstream>

namespace instrumentation {


print_visitor::print_visitor()
: print_visitor(std::cerr)
{}

print_visitor::~print_visitor() noexcept {}

auto print_visitor::operator()(const counter& c)
-> void {
  print_name_(c);
  print_tags_(c);
  out_ << " = " << *c << "\n";
}

auto print_visitor::operator()(const gauge<bool>& g)
-> void {
  print_name_(g);
  print_tags_(g);
  out_ << " = " << *g << "\n";
}

auto print_visitor::operator()(const gauge<std::int64_t>& g)
-> void {
  print_name_(g);
  print_tags_(g);
  out_ << " = " << *g << "\n";
}

auto print_visitor::operator()(const gauge<double>& g)
-> void {
  print_name_(g);
  print_tags_(g);
  out_ << " = " << *g << "\n";
}

auto print_visitor::operator()(const gauge<std::string>& g)
-> void {
  print_name_(g);
  print_tags_(g);
  out_ << " = " << *g << "\n";
}

auto print_visitor::operator()(const timing& t)
-> void {
  print_name_(t);
  print_tags_(t);
  out_ << " = {";
  bool first = true;
  for (timing::bucket b : t) {
#if 1 // Change to '__cplusplus < ...' when C++20 gets a version number.
    using tdelta = std::chrono::duration<double, std::milli>;
    tdelta lo(b.lo);
    tdelta hi(b.hi);

    out_ << (std::exchange(first, false) ? "" : ", ")
        << lo.count() << "ms" << "-" << hi.count() << "ms" << "=" << b.count;
#else
    out_ << (std::exchange(first, false) ? "" : ", ")
        << b.lo << "-" << b.hi << "=" << b.count;
#endif

  }
  out_ << "}\n";
}

auto print_visitor::print_name_(const hierarchy& h)
-> void {
  auto name = h.name();
  bool first = true;
  for (std::string_view name_element : name)
    out_ << (std::exchange(first, false) ? "" : ".") << name_element;
}

auto print_visitor::print_tags_(const hierarchy& h)
-> void {
  std::vector<std::pair<tags::map_type::key_type, tags::map_type::mapped_type>> values;
  {
    auto t = h.tags();
    std::copy(t.begin(), t.end(), std::back_inserter(values));
    std::sort(values.begin(), values.end(),
        [](const auto& x, const auto& y) { return x.first < y.first; });
  }

  bool first = true;
  out_ << "{";
  for (auto tag_value : values) {
    out_ << (std::exchange(first, false) ? "" : ",") << tag_value.first;
    std::visit(
        [this](const auto& v) {
          out_ << v;
        },
        tag_value.second);
  }
  out_ << "}";
}


} /* namespace instrumentation */
