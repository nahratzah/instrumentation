#include <instrumentation/prometheus.h>
#include <instrumentation/engine.h>
#include <instrumentation/counter.h>
#include <instrumentation/gauge.h>
#include <instrumentation/string.h>
#include <instrumentation/timing.h>
#include <UnitTest++/UnitTest++.h>
#include <string>
#include <chrono>

using namespace instrumentation;

TEST(prometheus_counter) {
  engine e;
  counter_vector<std::string> mv(e, "test.metric", {"label_name"}, "this is a test");
  mv.labels("foo") += 11;

  CHECK_EQUAL(std::string()
      + "# HELP test_metric this is a test\n"
      + "# TYPE test_metric counter\n"
      + "test_metric\t{label_name=\"foo\",}\t11\n",
      collect_prometheus(e));
}

TEST(prometheus_gauge) {
  engine e;
  gauge_vector<std::string> mv(e, "test.metric", {"label_name"}, "this is a test");
  mv.labels("foo") = 11.5;

  CHECK_EQUAL(std::string()
      + "# HELP test_metric this is a test\n"
      + "# TYPE test_metric gauge\n"
      + "test_metric\t{label_name=\"foo\",}\t11.5\n",
      collect_prometheus(e));
}

TEST(prometheus_string) {
  engine e;
  string_vector<std::string> mv(e, "test.metric", {"label_name"}, "this is a test");
  mv.labels("foo") = "text-value";

  CHECK_EQUAL(std::string()
      + "# HELP test_metric this is a test\n"
      + "# TYPE test_metric untyped\n"
      + "test_metric\t{label_name=\"foo\",strval=\"text-value\",}\t1\n",
      collect_prometheus(e));
}

TEST(prometheus_timing) {
  using namespace std::chrono_literals;

  engine e;
  timing_vector<std::string> mv(e, "test.metric", {"label_name"}, {1ms, 5ms, 300ms}, "this is a test");
  mv.labels("foo")
      << 300us
      << 9ms
      << 9ms
      << 7s
      << 8s
      << 3000h;

  CHECK_EQUAL(std::string()
      + "# HELP test_metric_seconds this is a test\n"
      + "# TYPE test_metric_seconds histogram\n"
      + "test_metric_seconds\t{label_name=\"foo\",le=\"0.001\",}\t1\n"
      + "test_metric_seconds\t{label_name=\"foo\",le=\"0.005\",}\t1\n"
      + "test_metric_seconds\t{label_name=\"foo\",le=\"0.3\",}\t3\n"
      + "test_metric_seconds\t{label_name=\"foo\",le=\"+Inf\",}\t6\n",
      collect_prometheus(e));
}

TEST(label_types) {
  engine e;
  counter_vector<bool, std::int64_t, double, std::string> mv(e, "test.metric", {"bool", "int", "double", "string"});
  mv.labels(true, 17, 19.0, "text") += 11;

  CHECK_EQUAL(std::string()
      + "# TYPE test_metric counter\n"
      + "test_metric\t{bool=\"true\",double=\"19\",int=\"17\",string=\"text\",}\t11\n",
      collect_prometheus(e));
}

TEST(no_labels) {
  engine e;
  counter_vector<> mv(e, "test.metric", {});
  mv.labels() += 11;

  CHECK_EQUAL(std::string()
      + "# TYPE test_metric counter\n"
      + "test_metric\t11\n",
      collect_prometheus(e));
}

TEST(multi_line_help) {
  engine e;
  counter_vector<> mv(e, "test.metric", {}, "this help text\nhas a second line of text");
  mv.labels() += 11;

  CHECK_EQUAL(std::string()
      + "# HELP test_metric this help text\\nhas a second line of text\n"
      + "# TYPE test_metric counter\n"
      + "test_metric\t11\n",
      collect_prometheus(e));
}

TEST(backslash_in_help) {
  engine e;
  counter_vector<> mv(e, "test.metric", {}, "the backslash \\ is an escape character");
  mv.labels() += 11;

  CHECK_EQUAL(std::string()
      + "# HELP test_metric the backslash \\\\ is an escape character\n"
      + "# TYPE test_metric counter\n"
      + "test_metric\t11\n",
      collect_prometheus(e));
}

TEST(label_escape) {
  engine e;
  counter_vector<std::string> mv(e, "test.metric", {"test"});
  mv.labels("newline \n, double quote \", and backslash \\ are escaped") += 11;

  CHECK_EQUAL(std::string()
      + "# TYPE test_metric counter\n"
      + "test_metric\t{test=\"newline \\n, double quote \\\", and backslash \\\\ are escaped\",}\t11\n",
      collect_prometheus(e));
}

TEST(fix_metric_name) {
  engine e;
  counter_vector<> mv(e, "The quick brown fox, what did they do?", {});
  mv.labels() += 11;

  CHECK_EQUAL(std::string()
      + "# TYPE The_quick_brown_fox__what_did_they_do_ counter\n"
      + "The_quick_brown_fox__what_did_they_do_\t11\n",
      collect_prometheus(e));
}

TEST(fix_label_name) {
  engine e;
  counter_vector<std::int64_t> mv(e, "test.metric", {".,?:;'\""});
  mv.labels(1);

  CHECK_EQUAL(std::string()
      + "# TYPE test_metric counter\n"
      + "test_metric\t{___:___=\"1\",}\t0\n",
      collect_prometheus(e));
}

int main() {
  return UnitTest::RunAllTests();
}
