#include <instrumentation/prometheus.h>
#include <instrumentation/collector.h>
#include <instrumentation/counter.h>
#include <instrumentation/gauge.h>
#include <instrumentation/string.h>
#include <instrumentation/timing.h>
#include <instrumentation/engine.h>
#include <instrumentation/metric_name.h>
#include <instrumentation/tags.h>
#include <cmath>
#include <cstdint>
#include <ios>
#include <iterator>
#include <map>
#include <optional>
#include <ostream>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

namespace instrumentation {
namespace {


class flag_manager {
  public:
  flag_manager() = default;

  flag_manager(std::ios_base& out)
  : stream_(&out)
  {
    saved_ = stream_->setf(std::ios_base::fixed, std::ios_base::floatfield);
  }

  ~flag_manager() noexcept {
    if (stream_ != nullptr) stream_->setf(saved_, std::ios_base::floatfield);
  }

  private:
  std::ios_base* stream_ = nullptr;
  std::ios_base::fmtflags saved_;
};


class prom_collector
: public collector
{
  public:
  prom_collector(std::ostream& out)
  : out(out)
  {}

  void visit_description(const metric_name& name, std::string_view description) override {
    pending_help.emplace(description.begin(), description.end());
  }

  void visit(const metric_name& name, const tags& t, const counter& c) override {
    write_(name, t, *c, "counter");
  }

  void visit(const metric_name& name, const tags& t, const gauge& g) override {
    write_(name, t, *g, "gauge");
  }

  void visit(const metric_name& name, const tags& t, const string& s) override {
    if (t.data().count("strval") == 0) {
      tags tag_copy = t;
      tag_copy.with("strval", *s);
      write_(name, t, 1.0, "untyped");
    }
  }

  void visit(const metric_name& name, const tags& t, const timing& m) override {
    const auto h = *m;
    metric_name decorated_name = name;
    decorated_name.data().push_back("seconds");

    tags tag_copy = t;
    std::uint64_t cumulative_count = 0;
    for (const timing::histogram_entry& he : std::get<0>(h)) {
      std::chrono::duration<double> d = he.le;
      tag_copy.with("le", d.count());

      cumulative_count += he.bucket_count;

      write_(decorated_name, tag_copy, cumulative_count, "histogram");
    }

    tag_copy.with("le", "+Inf");
    write_(name, tag_copy, cumulative_count + std::get<1>(h), "histogram");
  }

  private:
  template<typename T>
  void write_(const metric_name& name, const tags& t, const T& v, const char* metric_type = "untyped") {
    const auto pm_name = prom_metric_name(name);

    flag_manager fm{ out };

    if (pending_help) {
      out << "# TYPE " << pm_name << " " << metric_type << "\n";
      if (!pending_help->empty())
        out << "# HELP " << pm_name << " " << *pending_help << "\n";
      pending_help.reset();
    }

    out << pm_name << "\t";
    write_tags_(t);


    if constexpr(std::is_floating_point_v<T>) {
      // For floating point, ensure we handle the edge cases correctly.
      if (std::isnan(v)) {
        out << R"("NaN")";
      } else if (std::isinf(v)) {
        out << (v < 0 ? R"(-Inf)" : R"(+Inf)");
      } else {
        out << v;
      }
    } else {
      out << v;
    }

    out << "\n";
  }

  static auto prom_metric_name(const metric_name& name) -> std::string {
    return fix_prom_name(name.with_separator("_"));
  }

  void write_tags_(const tags& t) {
    if (t.empty()) return;

    std::map<std::string, std::string> tmp;
    for (const auto& e : t.data()) {
      std::string v_as_str = std::visit(
          [](const auto& v) -> std::string {
            if constexpr(std::is_same_v<bool, std::decay_t<decltype(v)>>) {
              return v ? R"("true")" : R"("false")";
            } else if constexpr(std::is_same_v<std::int64_t, std::decay_t<decltype(v)>>) {
              std::ostringstream oss;
              oss.setf(std::ios_base::dec, std::ios_base::basefield);
              oss << v;
              return quote_string(oss.str());
            } else if constexpr(std::is_same_v<double, std::decay_t<decltype(v)>>) {
              if (std::isnan(v)) {
                return R"("NaN")";
              } else if (std::isinf(v)) {
                return v < 0 ? R"(-Inf)" : R"(+Inf)";
              } else {
                std::ostringstream oss;
                oss.setf(std::ios_base::fixed, std::ios_base::floatfield);
                oss << v;
                return quote_string(oss.str());
              }
            } else {
              return quote_string(v);
            }
          },
          e.second);
      tmp.emplace(
          fix_prom_name(e.first),
          v_as_str);
    }

    out << "{";
    for (const auto& e : tmp) out << e.first << "=" << e.second << ",";
    out << "}\t";
  }

  static auto fix_prom_descr(std::string_view s) -> std::string {
    std::string out;
    out.reserve(s.size());

    for (const auto& c : s) {
      switch (c) {
        default:
          out.push_back(c);
          break;
        case '\\':
          out.append(R"(\\)");
          break;
        case '\n':
          out.append(R"(\n)");
          break;
      }
    }

    return out;
  }

  static auto fix_prom_name(std::string_view s) -> std::string {
    static const auto bad_start_characters = std::regex("[^a-zA-Z_:]");
    static const auto bad_tail_characters = std::regex("[^a-zA-Z0-9_:]");

    std::string out;
    out.reserve(s.size());

    if (!s.empty()) {
      std::regex_replace(
          std::back_inserter(out),
          s.begin(), s.begin() + 1u,
          bad_start_characters,
          "_");
      std::regex_replace(
          std::back_inserter(out),
          s.begin() + 1u, s.end(),
          bad_tail_characters,
          "_");
    }

    return out;
  }

  static auto quote_string(std::string_view s) -> std::string {
    static const auto escape_str_chars = std::regex(R"--(["\\\n])--");

    std::string out;
    out.reserve(s.size() + 2u);

    out.append(1, '"');
    std::regex_replace(
        std::back_inserter(out),
        s.begin(), s.end(),
        escape_str_chars,
        R"--(\\$1)--");
    out.append(1, '"');
    return out;
  }

  std::ostream& out;
  std::optional<std::string> pending_help;
};


} /* namespace instrumentation::<unnamed> */


void collect_prometheus(std::ostream& out) {
  return collect_prometheus(out, engine::global());
}

void collect_prometheus(std::ostream& out, const engine& e) {
  prom_collector pc(out);
  e.collect(pc);
}

auto collect_prometheus() -> std::string {
  std::ostringstream oss;
  collect_prometheus(oss);
  return std::move(oss).str();
}

auto collect_prometheus(const engine& e) -> std::string {
  std::ostringstream oss;
  collect_prometheus(oss, e);
  return std::move(oss).str();
}


} /* namespace instrumentation */
