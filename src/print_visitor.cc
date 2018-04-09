#include <instrumentation/print_visitor.h>
#include <instrumentation/counter.h>
#include <instrumentation/gauge.h>
#include <iostream>

namespace instrumentation {


print_visitor::print_visitor()
: print_visitor(std::cerr)
{}

print_visitor::~print_visitor() noexcept {}

auto print_visitor::operator()(const counter& c)
-> void {
  print_name_(c);
  print_tags_(c);
  out_ << " = " << *c;
}

auto print_visitor::operator()(const gauge<bool>& g)
-> void {
  print_name_(g);
  print_tags_(g);
  out_ << " = " << *g;
}

auto print_visitor::operator()(const gauge<std::int64_t>& g)
-> void {
  print_name_(g);
  print_tags_(g);
  out_ << " = " << *g;
}

auto print_visitor::operator()(const gauge<double>& g)
-> void {
  print_name_(g);
  print_tags_(g);
  out_ << " = " << *g;
}

auto print_visitor::operator()(const gauge<std::string>& g)
-> void {
  print_name_(g);
  print_tags_(g);
  out_ << " = " << *g;
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
