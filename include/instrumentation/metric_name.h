#ifndef INSTRUMENTATION_METRIC_NAME_H
#define INSTRUMENTATION_METRIC_NAME_H

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

#endif /* INSTRUMENTATION_METRIC_NAME_H */
