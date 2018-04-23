#include <instrumentation/print_visitor.h>
#include <instrumentation/counter.h>
#include <instrumentation/gauge.h>
#include <instrumentation/timing.h>
#include <instrumentation/timing_accumulate.h>
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
  using tdelta = std::chrono::duration<double, std::milli>;

  tdelta total_lo(0), total_hi(0);
  std::uint64_t total_count(0);

  print_name_(t);
  print_tags_(t);
  out_ << " = {";
  bool first = true;
  for (timing::bucket b : t) {
    if (b.count == 0) continue; // Don't print zeroes, they're distracting.

    tdelta lo(b.lo);
    tdelta hi(b.hi);

    total_count += b.count;
    total_lo += lo * b.count;
    total_hi += hi * b.count;

    out_ << (std::exchange(first, false) ? "" : ", ")
        << lo.count() << "ms" << "-" << hi.count() << "ms" << "=" << b.count;
  }
  out_ << "} (total "
      << total_lo.count() << "ms .. "
      << total_hi.count() << "ms over "
      << total_count << " events)\n";
}

auto print_visitor::operator()(const timing_accumulate& t)
-> void {
  print_name_(t);
  print_tags_(t);

#if 1 // Change to '__cplusplus < ...' when C++20 gets a version number.
  using tdelta = std::chrono::duration<double, std::milli>;

  out_ << " = " << tdelta(*t).count() << "ms\n";
#else
  out_ << " = " << *t << "\n";
#endif
}

auto print_visitor::print_name_(const basic_metric& h)
-> void {
  bool first = true;
  for (std::string_view name_element : h.name)
    out_ << (std::exchange(first, false) ? "" : ".") << name_element;
}

auto print_visitor::print_tags_(const basic_metric& h)
-> void {
  std::vector<std::pair<tag_map::key_type, tag_map::mapped_type>> values;
  {
    std::copy(h.tags.begin(), h.tags.end(), std::back_inserter(values));
    std::sort(values.begin(), values.end(),
        [](const auto& x, const auto& y) { return x.first < y.first; });
  }

  bool first = true;
  out_ << "{";
  for (auto tag_value : values) {
    out_ << (std::exchange(first, false) ? "" : ",") << tag_value.first << "=";
    std::visit(
        [this](const auto& v) {
          out_ << v;
        },
        tag_value.second);
  }
  out_ << "}";
}


} /* namespace instrumentation */
