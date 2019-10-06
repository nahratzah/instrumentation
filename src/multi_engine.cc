#include <instrumentation/multi_engine.h>
#include <memory>
#include <vector>

namespace instrumentation {
namespace {


class multi_engine_impl final
: public engine_intf
{
  private:
  class counter;
  class gauge;
  class string;
  class timing;

  public:
  multi_engine_impl(std::initializer_list<engine> init)
  : engines_(init)
  {}

  explicit multi_engine_impl(std::vector<engine> init)
  : engines_(init)
  {}

  auto new_counter(path p, tags t) -> std::shared_ptr<counter_intf> override;
  auto new_gauge(path p, tags t) -> std::shared_ptr<gauge_intf> override;
  auto new_string(path p, tags t) -> std::shared_ptr<string_intf> override;
  auto new_timing(path p, tags t, timing_intf::duration resolution, std::size_t buckets) -> std::shared_ptr<timing_intf> override;
  auto new_cumulative_timing(path p, tags t) -> std::shared_ptr<timing_intf> override;

  auto new_counter_cb(path p, tags t, std::function<double()> cb) -> std::shared_ptr<void> override;
  auto new_gauge_cb(path p, tags t, std::function<double()> cb) -> std::shared_ptr<void> override;
  auto new_string_cb(path p, tags t, std::function<std::string()> cb) -> std::shared_ptr<void> override;

  private:
  std::vector<engine> engines_;
};


class multi_engine_impl::counter final
: public counter_intf
{
  public:
  counter(std::vector<instrumentation::counter> counters)
  : counters_(std::move(counters))
  {}

  void do_inc(double d) noexcept override {
    for (instrumentation::counter& c : counters_) c += d;
  }

  private:
  std::vector<instrumentation::counter> counters_;
};


class multi_engine_impl::gauge final
: public gauge_intf
{
  public:
  gauge(std::vector<instrumentation::gauge> gauges)
  : gauges_(std::move(gauges))
  {}

  void do_inc(double d) noexcept override {
    for (instrumentation::gauge& g : gauges_) g += d;
  }

  void do_set(double d) noexcept override {
    for (instrumentation::gauge& g : gauges_) g = d;
  }

  private:
  std::vector<instrumentation::gauge> gauges_;
};


class multi_engine_impl::string final
: public string_intf
{
  public:
  string(std::vector<instrumentation::string> strings)
  : strings_(std::move(strings))
  {}

  void do_set(std::string_view str) noexcept override {
    for (instrumentation::string& s : strings_) s = str;
  }

  private:
  std::vector<instrumentation::string> strings_;
};


class multi_engine_impl::timing final
: public timing_intf
{
  public:
  timing(std::vector<instrumentation::timing> timings)
  : timings_(std::move(timings))
  {}

  void do_add(duration d) noexcept override {
    for (instrumentation::timing& s : timings_) s << d;
  }

  private:
  std::vector<instrumentation::timing> timings_;
};


auto multi_engine_impl::new_counter(path p, tags t) -> std::shared_ptr<counter_intf> {
  std::vector<instrumentation::counter> batch;
  batch.reserve(engines_.size());
  for (engine& e : engines_) batch.emplace_back(e.new_counter(p, t));
  return std::make_shared<counter>(std::move(batch));
}

auto multi_engine_impl::new_gauge(path p, tags t) -> std::shared_ptr<gauge_intf> {
  std::vector<instrumentation::gauge> batch;
  batch.reserve(engines_.size());
  for (engine& e : engines_) batch.emplace_back(e.new_gauge(p, t));
  return std::make_shared<gauge>(std::move(batch));
}

auto multi_engine_impl::new_string(path p, tags t) -> std::shared_ptr<string_intf> {
  std::vector<instrumentation::string> batch;
  batch.reserve(engines_.size());
  for (engine& e : engines_) batch.emplace_back(e.new_string(p, t));
  return std::make_shared<string>(std::move(batch));
}

auto multi_engine_impl::new_timing(path p, tags t, timing_intf::duration resolution, std::size_t buckets) -> std::shared_ptr<timing_intf> {
  std::vector<instrumentation::timing> batch;
  batch.reserve(engines_.size());
  for (engine& e : engines_) batch.emplace_back(e.new_timing(p, t, resolution, buckets));
  return std::make_shared<timing>(std::move(batch));
}

auto multi_engine_impl::new_cumulative_timing(path p, tags t) -> std::shared_ptr<timing_intf> {
  std::vector<instrumentation::timing> batch;
  batch.reserve(engines_.size());
  for (engine& e : engines_) batch.emplace_back(e.new_cumulative_timing(p, t));
  return std::make_shared<timing>(std::move(batch));
}

auto multi_engine_impl::new_counter_cb(path p, tags t, std::function<double()> cb) -> std::shared_ptr<void> {
  auto v = std::make_shared<std::vector<std::shared_ptr<void>>>();
  v->reserve(engines_.size());
  for (engine& e : engines_) v->emplace_back(e.new_counter_cb(p, t, cb));
  return v;
}

auto multi_engine_impl::new_gauge_cb(path p, tags t, std::function<double()> cb) -> std::shared_ptr<void> {
  auto v = std::make_shared<std::vector<std::shared_ptr<void>>>();
  v->reserve(engines_.size());
  for (engine& e : engines_) v->emplace_back(e.new_gauge_cb(p, t, cb));
  return v;
}

auto multi_engine_impl::new_string_cb(path p, tags t, std::function<std::string()> cb) -> std::shared_ptr<void> {
  auto v = std::make_shared<std::vector<std::shared_ptr<void>>>();
  v->reserve(engines_.size());
  for (engine& e : engines_) v->emplace_back(e.new_string_cb(p, t, cb));
  return v;
}


} /* namespace instrumentation::<unnamed> */


auto multi_engine(std::initializer_list<engine> engines) -> engine {
  return engine(std::make_shared<multi_engine_impl>(engines));
}

auto multi_engine(std::vector<engine> engines) -> engine {
  return engine(std::make_shared<multi_engine_impl>(engines));
}


} /* namespace instrumentation */
