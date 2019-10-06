#include <instrumentation/path.h>

namespace instrumentation {


path::path(std::string_view path) {
  if (!path.empty()) {
    for (auto pos = path.find("."); pos != std::string_view::npos; pos = path.find(".")) {
      elements.emplace_back(path.begin(), path.begin() + pos);
      path.remove_prefix(pos);
      path.remove_prefix(1);
    }
    elements.emplace_back(path.begin(), path.end());
  }
}

auto path::with_separator(std::string_view sep) const -> std::string {
  std::string result;
  const auto& d = data();

  auto iter = d.begin();
  if (iter != d.end()) {
    result.append(iter->begin(), iter->end());

    while (++iter != d.end()) {
      result.append(sep.begin(), sep.end());
      result.append(iter->begin(), iter->end());
    }
  }
  return result;
}


} /* namespace instrumentation */
