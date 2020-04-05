#include <instrumentation/engine.h>

namespace instrumentation {


auto engine::global() -> engine& {
  static engine impl_;
  return impl_;
}

void engine::collect(collector& c) const {
  std::shared_lock<std::shared_mutex> lck{ mtx_ };

  for (const auto& metric_pair : metrics_)
    metric_pair.second->collect(metric_pair.first, c);
}


} /* namespace instrumentation */
