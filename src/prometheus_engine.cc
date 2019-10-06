#include <instrumentation/prometheus_engine.h>
#include <algorithm>
#include <cmath>
#include <iterator>
#include <map>
#include <mutex>
#include <regex>
#include <sstream>
#include <variant>
#include <type_traits>

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


class atoms
: private std::allocator<std::atomic<std::uint64_t>>
{
  public:
  using size_type = std::size_t;
  using iterator = std::atomic<std::uint64_t>*;
  using const_iterator = const std::atomic<std::uint64_t>*;

  atoms() = default;

  atoms(size_type size)
  : size_(size),
    data_(this->allocate(this->size_))
  {}

  atoms(const atoms&) = delete;

  atoms(atoms&& y) noexcept
  : size_(y.size_),
    data_(std::exchange(y.data_, nullptr))
  {}

  atoms& operator=(const atoms&) = delete;

  atoms& operator=(atoms&& y) noexcept {
    using std::swap;
    swap(size_, y.size_);
    swap(data_, y.data_);
    return *this;
  }

  ~atoms() noexcept {
    if (this->data_ != nullptr) this->deallocate(this->data_, this->size_);
  }

  auto size() const noexcept {
    return size_;
  }

  auto operator[](size_type idx) -> std::atomic<std::uint64_t>& {
    assert(idx < size());
    return data_[idx];
  }

  auto operator[](size_type idx) const -> const std::atomic<std::uint64_t>& {
    assert(idx < size());
    return data_[idx];
  }

  auto begin() -> iterator {
    return data_;
  }

  auto end() -> iterator {
    return data_ + size_;
  }

  auto begin() const -> const_iterator {
    return data_;
  }

  auto end() const -> const_iterator {
    return data_ + size_;
  }

  auto cbegin() const -> const_iterator {
    return data_;
  }

  auto cend() const -> const_iterator {
    return data_ + size_;
  }

  private:
  size_type size_ = 0u;
  std::atomic<std::uint64_t>* data_ = nullptr;
};


} /* namespace instrumentation::<unnamed> */


class prometheus_engine::collector_intf {
  protected:
  virtual ~collector_intf() noexcept;

  collector_intf(prom_type type) noexcept
  : type(type)
  {}

  public:
  virtual void collect(std::ostream& out, const metric_name& name) const = 0;

  prom_type type;
};


class prometheus_engine::metric_collector final
: public collector_intf,
  public counter_intf,
  public gauge_intf
{
  public:
  ~metric_collector() noexcept override;

  metric_collector(prom_type type) noexcept
  : collector_intf(type)
  {}

  void collect(std::ostream& out, const metric_name& name) const override {
    flag_manager fm{ out };
    out << name.name;
    if (!name.tags.empty()) out << "\t{" << name.tags << "}";
    out << "\t" << v_.load(std::memory_order_relaxed);
  }

  private:
  void do_inc(double d) noexcept override {
    // Until C++20 makes it possible to use fetch_add, we'll use a loop.
    double expect = v_.load(std::memory_order_relaxed);
    while (!v_.compare_exchange_weak(expect, expect + d, std::memory_order_relaxed, std::memory_order_relaxed)) {
      // skip
    }
  }

  void do_set(double d) noexcept override {
    v_.store(d, std::memory_order_relaxed);
  }

  std::atomic<double> v_;
};


class prometheus_engine::timing_collector
: public collector_intf,
  public timing_intf
{
  public:
  ~timing_collector() noexcept override;

  timing_collector(duration resolution, std::size_t buckets)
  : collector_intf(prom_type::histogram),
    resolution_(resolution),
    buckets_(buckets)
  {}

  void collect(std::ostream& out, const metric_name& name) const override {
    using tdelta = std::chrono::duration<double>; // floating-point seconds

    flag_manager fm{ out };

    std::uint64_t cumulative_count = 0;

    // Emit each bucket.
    duration d{0};
    for (const auto& b : buckets_) {
      d += resolution_;
      cumulative_count += b.load(std::memory_order_relaxed);
      out << name.name
          << "\t{le=\"" << tdelta(d).count() << "\"," << name.tags << "}\t"
          << cumulative_count;
    }

    // Emit le=+Inf
    cumulative_count += inf_.load(std::memory_order_relaxed);
    out << name.name
        << "\t{le=\"+Inf\"," << name.tags << "}\t"
        << cumulative_count;

    // Emit _count metric.
    out << name.name << "_count";
    if (!name.tags.empty()) out << "\t{" << name.tags << "}\t";
    out << cumulative_count;

    // Emit _sum metric.
    // (We convert the sum to floating point seconds.)
    const auto sum_metric = tdelta(duration(sum_.load(std::memory_order_relaxed)));
    out << name.name << "_sum";
    if (!name.tags.empty()) out << "\t{" << name.tags << "}\t";
    out << sum_metric.count();
  }

  private:
  void do_add(duration d) noexcept override {
    const std::size_t idx = d / resolution_;
    if (idx < buckets_.size())
      buckets_[idx].fetch_add(1u, std::memory_order_relaxed);
    else
      inf_.fetch_add(1u, std::memory_order_relaxed);
    sum_.fetch_add(d.count(), std::memory_order_relaxed);
  }

  const duration resolution_;
  atoms buckets_;
  std::atomic<std::uint64_t> inf_ = 0;
  std::atomic<duration::rep> sum_{0};
};


class prometheus_engine::cumulative_timing_collector
: public collector_intf,
  public timing_intf
{
  public:
  ~cumulative_timing_collector() noexcept override;

  cumulative_timing_collector()
  : collector_intf(prom_type::counter)
  {}

  void collect(std::ostream& out, const metric_name& name) const override {
    using tdelta = std::chrono::duration<double>; // floating-point seconds

    flag_manager fm{ out };

    // (We convert the sum to floating point seconds.)
    const auto sum_metric = tdelta(duration(sum_.load(std::memory_order_relaxed)));
    out << name.name;
    if (!name.tags.empty()) out << "\t{" << name.tags << "}\t";
    out << sum_metric.count();
  }

  private:
  void do_add(duration d) noexcept override {
    sum_.fetch_add(d.count(), std::memory_order_relaxed);
  }

  std::atomic<duration::rep> sum_{0};
};


class prometheus_engine::functor_collector
: public collector_intf
{
  public:
  ~functor_collector() noexcept override;

  functor_collector(prom_type type, std::function<double()> cb)
  : collector_intf(type),
    cb_(std::move(cb))
  {}

  void collect(std::ostream& out, const metric_name& name) const override {
    if (cb_ != nullptr) {
      flag_manager fm{ out };
      out << name.name;
      if (!name.tags.empty()) out << "\t{" << name.tags << "}";
      out << "\t" << cb_();
    }
  }

  private:
  std::function<double()> cb_;
};


prometheus_engine::collector_intf::~collector_intf() noexcept = default;
prometheus_engine::metric_collector::~metric_collector() noexcept = default;
prometheus_engine::timing_collector::~timing_collector() noexcept = default;
prometheus_engine::cumulative_timing_collector::~cumulative_timing_collector() noexcept = default;
prometheus_engine::functor_collector::~functor_collector() noexcept = default;


auto prometheus_engine::fix_prom_name(std::string_view s) -> std::string {
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

auto prometheus_engine::quote_string(std::string_view s) -> std::string {
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

auto prometheus_engine::path_to_string(const path& p) -> std::string {
  return fix_prom_name(p.with_separator("_"));
}

auto prometheus_engine::tags_to_string(const tags& t) -> std::string {
  if (t.empty()) return "";

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

  std::ostringstream oss;
  for (const auto& e : tmp) oss << e.first << "=" << e.second << ",";
  return oss.str();
}

void prometheus_engine::add_help(const path& p, std::string help) {
  // Remove newline from help text.
  const auto newline_pos = help.find("\n");
  if (newline_pos != std::string::npos)
    help = help.substr(0, newline_pos);

  std::lock_guard<std::shared_mutex> lck{ mtx_ };

  help_.emplace(path_to_string(p), std::move(help));
}

auto prometheus_engine::new_counter(path p, tags t) -> std::shared_ptr<counter_intf> {
  auto result = std::make_shared<metric_collector>(prom_type::counter);

  std::lock_guard<std::shared_mutex> lck{ mtx_ };

  map_.emplace(
      std::piecewise_construct,
      std::forward_as_tuple(
          path_to_string(p),
          tags_to_string(t)),
      std::forward_as_tuple(result));
  return result;
}

auto prometheus_engine::new_gauge(path p, tags t) -> std::shared_ptr<gauge_intf> {
  auto result = std::make_shared<metric_collector>(prom_type::gauge);

  std::lock_guard<std::shared_mutex> lck{ mtx_ };

  map_.emplace(
      std::piecewise_construct,
      std::forward_as_tuple(
          path_to_string(p),
          tags_to_string(t)),
      std::forward_as_tuple(result));
  return result;
}

auto prometheus_engine::new_string([[maybe_unused]] path p, [[maybe_unused]] tags t) -> std::shared_ptr<string_intf> {
  return nullptr;
}

auto prometheus_engine::new_timing(path p, tags t, timing_intf::duration resolution, std::size_t buckets) -> std::shared_ptr<timing_intf> {
  auto result = std::make_shared<timing_collector>(resolution, buckets);

  std::lock_guard<std::shared_mutex> lck{ mtx_ };

  map_.emplace(
      std::piecewise_construct,
      std::forward_as_tuple(
          path_to_string(p),
          tags_to_string(t)),
      std::forward_as_tuple(result));
  return result;
}

auto prometheus_engine::new_cumulative_timing(path p, tags t) -> std::shared_ptr<timing_intf> {
  auto result = std::make_shared<cumulative_timing_collector>();

  std::lock_guard<std::shared_mutex> lck{ mtx_ };

  map_.emplace(
      std::piecewise_construct,
      std::forward_as_tuple(
          path_to_string(p),
          tags_to_string(t)),
      std::forward_as_tuple(result));
  return result;
}

auto prometheus_engine::new_counter_cb(path p, tags t, std::function<double()> cb) -> std::shared_ptr<void> {
  auto result = std::make_shared<functor_collector>(prom_type::counter, std::move(cb));

  std::lock_guard<std::shared_mutex> lck{ mtx_ };

  map_.emplace(
      std::piecewise_construct,
      std::forward_as_tuple(
          path_to_string(p),
          tags_to_string(t)),
      std::forward_as_tuple(result));
  return result;
}

auto prometheus_engine::new_gauge_cb(path p, tags t, std::function<double()> cb) -> std::shared_ptr<void> {
  auto result = std::make_shared<functor_collector>(prom_type::gauge, std::move(cb));

  std::lock_guard<std::shared_mutex> lck{ mtx_ };

  map_.emplace(
      std::piecewise_construct,
      std::forward_as_tuple(
          path_to_string(p),
          tags_to_string(t)),
      std::forward_as_tuple(result));
  return result;
}

auto prometheus_engine::new_string_cb([[maybe_unused]] path p, [[maybe_unused]] tags t, [[maybe_unused]] std::function<std::string()> cb) -> std::shared_ptr<void> {
  return nullptr;
}

void prometheus_engine::collect(std::ostream& out) const {
  flag_manager fm{ out };
  std::shared_lock<std::shared_mutex> lck{ mtx_ };
  bool must_run_maintenance = false;

  auto map_iter = map_.begin();
  std::vector<std::pair<const metric_name*, std::shared_ptr<collector_intf>>> collectors;
  while (map_iter != map_.end()) {
    // Figure out which metrics all share a name.
    auto range_end = std::find_if_not(
        std::next(map_iter), map_.end(),
        [&map_iter](const auto& e) -> bool {
          return metric_name_eq()(e.first, map_iter->first);
        });

    // Lock down all collectors.
    // If any can't be locked down, we omit it and mark this as requiring maintenance.
    collectors.clear();
    std::transform(
        map_iter, range_end,
        std::back_inserter(collectors),
        [](const auto& pair) {
          return std::make_pair(&pair.first, pair.second.lock());
        });
    const auto nil_begin = std::remove_if(
        collectors.begin(), collectors.end(),
        [](const auto& pair) {
          return pair.second == nullptr;
        });
    if (nil_begin != collectors.end()) {
      must_run_maintenance = true;
      collectors.erase(nil_begin, collectors.end());
    }

    // Update map_iter for next round.
    map_iter = range_end;
    if (collectors.empty()) continue;

    // If we have help text available, emit that.
    const auto help = help_.find(collectors.front().first->name);
    if (help != help_.end())
      out << "# HELP " << help->first << " " << help->second << "\n";

    // Emit the type.
    prom_type t = collectors.front().second->type;
    if (std::any_of(
            std::next(collectors.begin()), collectors.end(),
            [t](const auto& e) -> bool {
              return e.second->type != t;
            }))
      t = prom_type::untyped;
    out << "# TYPE " << collectors.front().first->name << " " << t << "\n";

    for (const auto& c : collectors) c.second->collect(out, *c.first);
  }

  if (must_run_maintenance) {
    lck.unlock();
    maintenance();
  }
}

auto prometheus_engine::collect() const -> std::string {
  std::ostringstream oss;
  collect(oss);
  return oss.str();
}

void prometheus_engine::maintenance() const {
  std::lock_guard<std::shared_mutex> lck{ mtx_ };

  auto map_iter = map_.begin();
  while (map_iter != map_.end()) {
    if (map_iter->second.lock()) {
      ++map_iter;
    } else {
      map_iter = map_.erase(map_iter);
    }
  }
}

auto operator<<(std::ostream& out, prometheus_engine::prom_type t) -> std::ostream& {
  switch (t) {
    default:
      [[fallthrough]];
    case prometheus_engine::prom_type::untyped:
      out << "untyped";
      break;
    case prometheus_engine::prom_type::counter:
      out << "counter";
      break;
    case prometheus_engine::prom_type::gauge:
      out << "gauge";
      break;
    case prometheus_engine::prom_type::histogram:
      out << "histogram";
      break;
    case prometheus_engine::prom_type::summary:
      out << "summary";
      break;
  }
  return out;
}


static_assert(!std::is_abstract_v<prometheus_engine>);


} /* namespace instrumentation */
