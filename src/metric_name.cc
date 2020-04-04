#include <instrumentation/metric_name.h>

namespace instrumentation {


metric_name::metric_name(std::string_view path) {
  if (!path.empty()) {
    for (auto pos = path.find("."); pos != std::string_view::npos; pos = path.find(".")) {
      elements.emplace_back(path.begin(), path.begin() + pos);
      path.remove_prefix(pos);
      path.remove_prefix(1);
    }
    elements.emplace_back(path.begin(), path.end());
  }
}

auto metric_name::with_separator(std::string_view sep) const -> std::string {
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

namespace std {


auto hash<instrumentation::metric_name>::operator()(const instrumentation::metric_name& name) const noexcept -> std::size_t {
  const std::hash<std::string> inner_hash;

  std::size_t result = 0;
  for (const auto& elem : name.data())
    result = 19u * result + inner_hash(elem);

  return result;
}


} /* namespace std */
