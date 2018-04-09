#include <instrumentation/group.h>
#include <algorithm>
#include <utility>

namespace instrumentation {


group::~group() noexcept {
  disable();
}

auto group::add(hierarchy& g)
noexcept
-> void {
  enable();
  std::lock_guard<std::mutex> lck{ mtx_ };
  g.sibling_ = std::exchange(child_, &g);
}

auto group::erase(hierarchy& g)
noexcept
-> void {
  std::lock_guard<std::mutex> lck{ mtx_ };
  for (hierarchy** child_iter = &child_;
      *child_iter != nullptr;
      child_iter = &(*child_iter)->sibling_) {
    if (*child_iter == &g) {
      *child_iter = std::exchange(g.sibling_, nullptr);
      return;
    }
  }
}

auto group::visit(visitor& v) const
-> void {
  v(*this);
}


} /* namespace instrumentation */
