#include <instrumentation/timing.h>
#include <instrumentation/engine.h>
#include <UnitTest++/UnitTest++.h>
#include "test_collector.h"
#include "print.h"
#include <string>

using namespace instrumentation;
using namespace std::chrono_literals;

TEST(default_constructor_creates_no_metric) {
  timing_vector<> tv;
  timing t = tv.labels();

  CHECK_EQUAL(true, !tv);
  CHECK_EQUAL(false, bool(tv));

  CHECK_EQUAL(true, !t);
  CHECK_EQUAL(false, bool(t));

  CHECK(std::get<0>(*t).empty());
  CHECK_EQUAL(0u, std::get<1>(*t));
}

TEST(no_metric_timing_ops_have_no_effect) {
  timing t;

  REQUIRE CHECK(std::get<0>(*t).empty());
  REQUIRE CHECK_EQUAL(0u, std::get<1>(*t));

  t << 0ms << 1ms << 5s;
  CHECK(std::get<0>(*t).empty());
  CHECK_EQUAL(0u, std::get<1>(*t));
}

TEST(timing_ops) {
  engine e;
  timing t = timing_vector<>(e, "test.metric", {}, {3s, 5s}, "").labels();

  REQUIRE CHECK_EQUAL(
      std::vector<timing::histogram_entry>({
            { 3s, 0 },
            { 5s, 0 },
          }),
      std::get<0>(*t));
  REQUIRE CHECK_EQUAL(0u, std::get<1>(*t));

  t << 1s << 2s << 3s << 4s << 5s << 6s;
  REQUIRE CHECK_EQUAL(
      std::vector<timing::histogram_entry>({
            { 3s, 3 }, // 1s, 2s, 3s
            { 5s, 2 }, // 4s, 5s
          }),
      std::get<0>(*t));
  REQUIRE CHECK_EQUAL(1u, std::get<1>(*t)); // 6s
}

TEST(timing_vector) {
  engine e;
  timing_vector<std::string> tv(e, "test.metric", {"label_name"}, {3s, 5s}, "this is a test");

  tv.labels("front") << 1s;
  tv.labels("mid") << 4s;
  tv.labels("tail") << 100s;

  CHECK_EQUAL(
      test_collector(
          { {"test.metric", "this is a test"} },
          { {"test.metric{label_name=\"front\"}", "[" + std::to_string(3.0) + "==>1, " + std::to_string(5.0) + "==>0, +Inf==>0]"},
            {"test.metric{label_name=\"mid\"}",   "[" + std::to_string(3.0) + "==>0, " + std::to_string(5.0) + "==>1, +Inf==>0]"},
            {"test.metric{label_name=\"tail\"}",  "[" + std::to_string(3.0) + "==>0, " + std::to_string(5.0) + "==>0, +Inf==>1]"},
          }),
      test_collector(e));
}

int main() {
  return UnitTest::RunAllTests();
}
