#ifndef INSTRUMENTATION_TAGS_H
#define INSTRUMENTATION_TAGS_H

#include <initializer_list>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <variant>

namespace instrumentation {


class tags {
  public:
  using tag_value = std::variant<bool, std::int64_t, double, std::string>;

  tags() = default;

  tags(std::initializer_list<std::pair<const std::string, tag_value>> init);

  template<typename T>
  auto with(std::string_view name, T&& value) & -> tags&;
  template<typename T>
  auto with(std::string_view name, T&& value) && -> tags&&;

  auto data() const noexcept -> const std::unordered_map<std::string, tag_value>&;
  auto empty() const noexcept -> bool;

  private:
  std::unordered_map<std::string, tag_value> tags_;
};


inline tags::tags(std::initializer_list<std::pair<const std::string, tag_value>> init)
: tags_(init)
{}

template<typename T>
inline auto tags::with(std::string_view name, T&& value) & -> tags& {
  if constexpr(std::is_same_v<bool, std::decay_t<T>>) {
    tags_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(name.begin(), name.end()),
        std::forward_as_tuple(std::in_place_type<bool>, value));
  } else if constexpr(std::is_integral_v<T>) {
    tags_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(name.begin(), name.end()),
        std::forward_as_tuple(std::in_place_type<std::int64_t>, value));
  } else if constexpr(std::is_floating_point_v<T>) {
    tags_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(name.begin(), name.end()),
        std::forward_as_tuple(std::in_place_type<std::int64_t>, value));
  } else {
    tags_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(name.begin(), name.end()),
        std::forward_as_tuple(std::in_place_type<std::string>, value));
  }
  return *this;
}

template<typename T>
auto tags::with(std::string_view name, T&& value) && -> tags&& {
  with(std::move(name), std::move(value));
  return std::move(*this);
}

inline auto tags::data() const noexcept -> const std::unordered_map<std::string, tag_value>& {
  return tags_;
}

inline auto tags::empty() const noexcept -> bool {
  return data().empty();
}


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_TAGS_H */
