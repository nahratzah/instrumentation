#ifndef INSTRUMENTATION_PATH_H
#define INSTRUMENTATION_PATH_H

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
class path {
  public:
  ///\brief Construct an empty path.
  path() = default;

  ///\brief Construct a path with the given path elements.
  path(std::initializer_list<std::string> elements);

  /**
   * \brief Construct a path from a strring.
   * \details
   * The \p path should contain one or more path elements, separated by a dot (`"."`).
   *
   * If the \p path is empty, an empty path will be constructed.
   *
   * \param path String representation of a path.
   */
  path(std::string_view path);

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
