#ifndef TEST_COLLECTOR_H
#define TEST_COLLECTOR_H

#include <instrumentation/collector.h>
#include <instrumentation/metric_name.h>
#include <instrumentation/tags.h>
#include <instrumentation/counter.h>
#include <instrumentation/gauge.h>
#include <instrumentation/string.h>
#include <instrumentation/timing.h>
#include <map>
#include <string>
#include <tuple>
#include <vector>

class test_collector final
: public instrumentation::collector
{
  public:
  test_collector() = default;

  test_collector(
      std::multimap<std::string, std::string> descriptions,
      std::multimap<std::string, std::string> metrics)
  : descriptions(std::move(descriptions)),
    metrics(std::move(metrics))
  {}

  explicit test_collector(const instrumentation::engine& e)
  : test_collector()
  {
    e.collect(*this);
  }

  void visit_description(const instrumentation::metric_name& n, std::string_view description) override {
    descriptions.emplace(to_string_(n), description);
  }

  void visit(const instrumentation::metric_name& n, const instrumentation::tags& t, const instrumentation::counter& m) override {
    metrics.emplace(to_string_(n) + to_string_(t), val_to_string_(*m));
  }

  void visit(const instrumentation::metric_name& n, const instrumentation::tags& t, const instrumentation::gauge& m) override {
    metrics.emplace(to_string_(n) + to_string_(t), val_to_string_(*m));
  }

  void visit(const instrumentation::metric_name& n, const instrumentation::tags& t, const instrumentation::string& m) override {
    metrics.emplace(to_string_(n) + to_string_(t), val_to_string_(*m));
  }

  void visit(const instrumentation::metric_name& n, const instrumentation::tags& t, const instrumentation::timing& m) override {
    metrics.emplace(to_string_(n) + to_string_(t), val_to_string_(*m));
  }

  private:
  static auto to_string_(const instrumentation::metric_name& m) -> std::string {
    return m.with_separator();
  }

  static auto to_string_(const instrumentation::tags& t) -> std::string {
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

  static auto val_to_string_(bool b) -> std::string {
    return (b ? "true" : "false");
  }

  static auto val_to_string_(std::int64_t v) -> std::string {
    return std::to_string(v);
  }

  static auto val_to_string_(std::uint64_t v) -> std::string {
    return std::to_string(v);
  }

  static auto val_to_string_(double v) -> std::string {
    return std::to_string(v);
  }

  static auto val_to_string_(const std::string& v) -> std::string {
    return "\"" + v + "\"";
  }

  static auto val_to_string_(const std::tuple<std::vector<instrumentation::timing::histogram_entry>, std::uint64_t>& h) -> std::string {
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

  public:
  std::multimap<std::string, std::string> descriptions;
  std::multimap<std::string, std::string> metrics;
};

inline auto operator<<(std::ostream& out, const test_collector& tc) -> std::ostream& {
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

inline auto operator==(const test_collector& x, const test_collector& y) noexcept -> bool {
  return x.descriptions == y.descriptions
      && x.metrics == y.metrics;
}

inline auto operator!=(const test_collector& x, const test_collector& y) noexcept -> bool {
  return !(x == y);
}

#endif /* TEST_COLLECTOR_H */
