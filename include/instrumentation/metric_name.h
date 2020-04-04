#ifndef INSTRUMENTATION_METRIC_NAME_H
#define INSTRUMENTATION_METRIC_NAME_H

#include <cstdint>
#include <functional>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

namespace instrumentation {


/**
 * \brief Models a metric name.
 * \details
 * A metric name is a sequence of path elements, forming a path.
 */
class metric_name {
  public:
  ///\brief Construct an empty path.
  metric_name() = default;

  ///\brief Construct a path with the given path elements.
  metric_name(std::initializer_list<std::string> elements);

  /**
   * \brief Construct a path from a strring.
   * \details
   * The \p path should contain one or more path elements, separated by a dot (`"."`).
   *
   * If the \p path is empty, an empty path will be constructed.
   *
   * \param path String representation of a path.
   */
  metric_name(std::string_view path);

  ///\brief Render the path using the given separator between path components.
  auto with_separator(std::string_view sep = ".") const -> std::string;
  ///\brief Test if this is an empty path.
  auto empty() const noexcept -> bool;
  ///\brief Inspect the underlying vector of path components.
  auto data() const noexcept -> const std::vector<std::string>&;
  ///\brief Allow modification of underlying path components.
  auto data() noexcept -> std::vector<std::string>&;

  auto operator==(const metric_name& y) const noexcept { return elements == y.elements; }
  auto operator!=(const metric_name& y) const noexcept { return !(*this == y); }

  private:
  ///\brief Sequence of path elements.
  std::vector<std::string> elements;
};


inline metric_name::metric_name(std::initializer_list<std::string> elements)
: elements(elements)
{}

inline auto metric_name::empty() const noexcept -> bool {
  return data().empty();
}

inline auto metric_name::data() const noexcept -> const std::vector<std::string>& {
  return elements;
}


} /* namespace instrumentation */

namespace std {


template<>
struct hash<instrumentation::metric_name> {
  // These are deprecated in c++17.
  using argument_type = instrumentation::metric_name;
  using result_type = std::size_t;

  auto operator()(const instrumentation::metric_name& name) const noexcept -> std::size_t;
};


} /* namespace std */

#endif /* INSTRUMENTATION_METRIC_NAME_H */
