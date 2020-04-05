#include <instrumentation/gauge.h>
#include <instrumentation/engine.h>
#include <UnitTest++/UnitTest++.h>
#include "test_collector.h"
#include <string>

using namespace instrumentation;

TEST(default_constructor_creates_no_metric) {
  gauge_vector<> gv;
  gauge g = gv.labels();

  CHECK_EQUAL(true, !gv);
  CHECK_EQUAL(false, bool(gv));

  CHECK_EQUAL(true, !g);
  CHECK_EQUAL(false, bool(g));

  CHECK_EQUAL(0.0, *g);
}

TEST(no_metric_gauge_ops_have_no_effect) {
  gauge g;

  REQUIRE CHECK_EQUAL(0.0, *g);
  ++g;
  CHECK_EQUAL(0.0, *g);
  --g;
  CHECK_EQUAL(0.0, *g);
}

TEST(gauge_ops) {
  engine e;
  gauge g = gauge_vector<>(e, "test.metric", {}).labels();

  REQUIRE CHECK_EQUAL(0.0, *g);
  ++g;
  CHECK_EQUAL(1.0, *g);
  g++;
  CHECK_EQUAL(2.0, *g);
  g += 17;
  CHECK_EQUAL(19.0, *g);
  g = 3;
  CHECK_EQUAL(3.0, *g);
  --g;
  CHECK_EQUAL(2.0, *g);
  g--;
  CHECK_EQUAL(1.0, *g);
}

TEST(gauge_vector) {
  engine e;
  gauge_vector<std::string> gv(e, "test.metric", {"label_name"}, "this is a test");

  gv.labels("foo") = 11;
  gv.labels("bar") = 17;

  CHECK_EQUAL(
      test_collector(
          { {"test.metric", "this is a test"} },
          { {"test.metric{label_name=\"foo\"}", std::to_string(11.0)},
            {"test.metric{label_name=\"bar\"}", std::to_string(17.0)}
          }),
      test_collector(e));
}

int main() {
  return UnitTest::RunAllTests();
}
