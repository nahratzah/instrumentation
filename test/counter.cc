#include <instrumentation/counter.h>
#include <instrumentation/engine.h>
#include <UnitTest++/UnitTest++.h>
#include "test_collector.h"
#include <string>

using namespace instrumentation;

TEST(default_constructor_creates_no_metric) {
  counter_vector<> cv;
  counter c = cv.labels();

  CHECK_EQUAL(true, !cv);
  CHECK_EQUAL(false, bool(cv));

  CHECK_EQUAL(true, !c);
  CHECK_EQUAL(false, bool(c));

  CHECK_EQUAL(0.0, *c);
}

TEST(no_metric_counter_ops_have_no_effect) {
  counter c;

  REQUIRE CHECK_EQUAL(0.0, *c);
  ++c;
  CHECK_EQUAL(0.0, *c);
}

TEST(counter_ops) {
  engine e;
  counter c = counter_vector<>(e, "test.metric", {}).labels();

  REQUIRE CHECK_EQUAL(0.0, *c);
  ++c;
  CHECK_EQUAL(1.0, *c);
  c++;
  CHECK_EQUAL(2.0, *c);
  c += 17;
  CHECK_EQUAL(19.0, *c);
}

TEST(counter_vector) {
  engine e;
  counter_vector<std::string> cv(e, "test.metric", {"label_name"}, "this is a test");

  cv.labels("foo") += 11;
  cv.labels("bar") += 17;

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
