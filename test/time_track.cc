#include <instrumentation/time_track.h>
#include <instrumentation/engine.h>
#include <instrumentation/timing.h>
#include <chrono>
#include <thread>
#include <vector>
#include <UnitTest++/UnitTest++.h>
#include "print.h"

using namespace instrumentation;
using namespace std::chrono_literals;

struct fake_metric {
  using clock_type = std::chrono::high_resolution_clock;
  using duration = clock_type::duration;

  fake_metric& operator<<(duration d) {
    ++invocations;
    value = d;
    return *this;
  }

  int invocations = 0;
  duration value;
};

TEST(works_with_timing) {
  engine e;
  timing t = timing_vector<>(e, "test.metric", {}, {}, "").labels();

  {
    time_track<timing> tt(t);
    std::this_thread::sleep_for(100ms);
  }

  CHECK_EQUAL(1u, std::get<1>(*t));
}

TEST(measure_duration) {
  fake_metric t;

  std::chrono::time_point<fake_metric::clock_type> inner_t0, inner_t1, outer_t0, outer_t1;
  outer_t0 = fake_metric::clock_type::now();
  {
    time_track<fake_metric> tt(t);
    inner_t0 = fake_metric::clock_type::now();
    std::this_thread::sleep_for(100ms);
    inner_t1 = fake_metric::clock_type::now();
  }
  outer_t1 = fake_metric::clock_type::now();

  const auto inner_duration = inner_t1 - inner_t0;
  const auto outer_duration = outer_t1 - outer_t0;

  CHECK_EQUAL(1, t.invocations);
  CHECK(t.value >= inner_duration);
  CHECK(t.value <= outer_duration);
}

TEST(do_untracked_suspends_measurement) {
  fake_metric t;

  std::chrono::time_point<fake_metric::clock_type> inner_t0, inner_t1, outer_t0, outer_t1;
  outer_t0 = fake_metric::clock_type::now();
  {
    time_track<fake_metric> tt(t);
    tt.do_untracked(
        [&]() {
          inner_t0 = fake_metric::clock_type::now();
          std::this_thread::sleep_for(200ms);
          inner_t1 = fake_metric::clock_type::now();
        });
  }
  outer_t1 = fake_metric::clock_type::now();

  const auto inner_duration = inner_t1 - inner_t0;
  const auto outer_duration = outer_t1 - outer_t0;

  CHECK_EQUAL(1, t.invocations);
  CHECK(t.value <= outer_duration - inner_duration);
}

int main() {
  return UnitTest::RunAllTests();
}
