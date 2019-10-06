#ifndef INSTRUMENTATION_PATH_H
#define INSTRUMENTATION_PATH_H

#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

namespace instrumentation {


class path {
  public:
  path() = default;

  path(std::initializer_list<std::string> elements);

  path(std::string_view path);

  auto with_separator(std::string_view sep = ".") const -> std::string;
  auto empty() const noexcept -> bool;
  auto data() const noexcept -> const std::vector<std::string>&;

  private:
  std::vector<std::string> elements;
};


inline path::path(std::initializer_list<std::string> elements)
: elements(elements)
{}

inline auto path::empty() const noexcept -> bool {
  return data().empty();
}

inline auto path::data() const noexcept -> const std::vector<std::string>& {
  return elements;
}


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_PATH_H */
