#include "test_collector.h"
#include <instrumentation/counter.h>
#include <instrumentation/gauge.h>
#include <instrumentation/string.h>
#include <instrumentation/timing.h>
#include <ostream>

test_collector::~test_collector() noexcept = default;

void test_collector::visit_description(const instrumentation::metric_name& n, std::string_view description) {
  descriptions.emplace(to_string_(n), description);
}

void test_collector::visit(const instrumentation::metric_name& n, const instrumentation::tags& t, const instrumentation::counter& m) {
  metrics.emplace(to_string_(n) + to_string_(t), val_to_string_(*m));
}

void test_collector::visit(const instrumentation::metric_name& n, const instrumentation::tags& t, const instrumentation::gauge& m) {
  metrics.emplace(to_string_(n) + to_string_(t), val_to_string_(*m));
}

void test_collector::visit(const instrumentation::metric_name& n, const instrumentation::tags& t, const instrumentation::string& m) {
  metrics.emplace(to_string_(n) + to_string_(t), val_to_string_(*m));
}

void test_collector::visit(const instrumentation::metric_name& n, const instrumentation::tags& t, const instrumentation::timing& m) {
  metrics.emplace(to_string_(n) + to_string_(t), val_to_string_(*m));
}

auto test_collector::to_string_(const instrumentation::metric_name& m) -> std::string {
  return m.with_separator();
}

auto test_collector::to_string_(const instrumentation::tags& t) -> std::string {
  std::map<std::string, std::string> labels;
  for (const auto& tval : t.data()) {
    labels[tval.first] = std::visit(
        [](const auto& v) {
          return val_to_string_(v);
        },
        tval.second);
  }

  std::string out = "{";
  bool first = true;
  for (const auto& label_pair : labels) {
    if (first)
      first = false;
    else
      out += ", ";

    out += label_pair.first;
    out += "=";
    out += label_pair.second;
  }
  out += "}";
  return out;
}

auto test_collector::val_to_string_(bool b) -> std::string {
  return (b ? "true" : "false");
}

auto test_collector::val_to_string_(std::int64_t v) -> std::string {
  return std::to_string(v);
}

auto test_collector::val_to_string_(std::uint64_t v) -> std::string {
  return std::to_string(v);
}

auto test_collector::val_to_string_(double v) -> std::string {
  return std::to_string(v);
}

auto test_collector::val_to_string_(const std::string& v) -> std::string {
  return "\"" + v + "\"";
}

auto test_collector::val_to_string_(const std::tuple<std::vector<instrumentation::timing::histogram_entry>, std::uint64_t>& h) -> std::string {
  std::string out = "[";
  for (const auto& h_entry : std::get<0>(h)) {
    std::chrono::duration<double> seconds = h_entry.le;
    out += val_to_string_(seconds.count());
    out += "==>";
    out += val_to_string_(h_entry.bucket_count);
    out += ", ";
  }
  out += "+Inf==>";
  out += val_to_string_(std::get<1>(h));
  out += "]";
  return out;
}

auto operator<<(std::ostream& out, const test_collector& tc) -> std::ostream& {
  out << "{\n";

  if (!tc.descriptions.empty()) {
    out << "  descriptions = [\n";
    for (const auto& description_pair : tc.descriptions) {
      out << "    " << description_pair.first << "="
          << description_pair.second << ",\n";
    }
    out << "  ],\n";
  }

  if (tc.metrics.empty()) {
    out << "  metrics = [],\n";
  } else {
    out << "  metrics = [\n";
    for (const auto& metric_pair : tc.metrics) {
      out << "    " << metric_pair.first << "=" << metric_pair.second << ",\n";
    }
    out << "  ],\n";
  }

  return out << "}";
}

auto operator==(const test_collector& x, const test_collector& y) noexcept -> bool {
  return x.descriptions == y.descriptions
      && x.metrics == y.metrics;
}
