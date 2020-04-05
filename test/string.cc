#include <instrumentation/string.h>
#include <instrumentation/engine.h>
#include <UnitTest++/UnitTest++.h>
#include "test_collector.h"
#include <string>

using namespace instrumentation;

TEST(default_constructor_creates_no_metric) {
  string_vector<> sv;
  string s = sv.labels();

  CHECK_EQUAL(true, !sv);
  CHECK_EQUAL(false, bool(sv));

  CHECK_EQUAL(true, !s);
  CHECK_EQUAL(false, bool(s));

  CHECK_EQUAL("", *s);
}

TEST(no_metric_string_ops_have_no_effect) {
  string s;

  REQUIRE CHECK_EQUAL("", *s);
  s = "bla bla chocoladevla";
  CHECK_EQUAL("", *s);
}

TEST(string_ops) {
  engine e;
  string s = string_vector<>(e, "test.metric", {}).labels();

  REQUIRE CHECK_EQUAL("", *s);
  s = "bla bla chocoladevla";
  CHECK_EQUAL("bla bla chocoladevla", *s);
}

TEST(string_vector) {
  engine e;
  string_vector<std::string> sv(e, "test.metric", {"label_name"}, "this is a test");

  sv.labels("foo") = "foo_value";
  sv.labels("bar") = "bar_value";

  CHECK_EQUAL(
      test_collector(
          { {"test.metric", "this is a test"} },
          { {"test.metric{label_name=\"foo\"}", R"("foo_value")"},
            {"test.metric{label_name=\"bar\"}", R"("bar_value")"}
          }),
      test_collector(e));
}

int main() {
  return UnitTest::RunAllTests();
}
