#ifndef TEST_COLLECTOR_H
#define TEST_COLLECTOR_H

#include <instrumentation/collector.h>
#include <instrumentation/metric_name.h>
#include <instrumentation/tags.h>
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

  ~test_collector() noexcept;

  void visit_description(const instrumentation::metric_name& n, std::string_view description) override;
  void visit(const instrumentation::metric_name& n, const instrumentation::tags& t, const instrumentation::counter& m) override;
  void visit(const instrumentation::metric_name& n, const instrumentation::tags& t, const instrumentation::gauge& m) override;
  void visit(const instrumentation::metric_name& n, const instrumentation::tags& t, const instrumentation::string& m) override;
  void visit(const instrumentation::metric_name& n, const instrumentation::tags& t, const instrumentation::timing& m) override;

  private:
  static auto to_string_(const instrumentation::metric_name& m) -> std::string;
  static auto to_string_(const instrumentation::tags& t) -> std::string;

  static auto val_to_string_(bool b) -> std::string;
  static auto val_to_string_(std::int64_t v) -> std::string;
  static auto val_to_string_(std::uint64_t v) -> std::string;
  static auto val_to_string_(double v) -> std::string;
  static auto val_to_string_(const std::string& v) -> std::string;
  static auto val_to_string_(const std::tuple<std::vector<instrumentation::timing::histogram_entry>, std::uint64_t>& h) -> std::string;

  public:
  std::multimap<std::string, std::string> descriptions;
  std::multimap<std::string, std::string> metrics;
};

auto operator<<(std::ostream& out, const test_collector& tc) -> std::ostream&;

auto operator==(const test_collector& x, const test_collector& y) noexcept -> bool;

inline auto operator!=(const test_collector& x, const test_collector& y) noexcept -> bool {
  return !(x == y);
}

#endif /* TEST_COLLECTOR_H */
