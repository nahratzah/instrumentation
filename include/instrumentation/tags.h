#ifndef INSTRUMENTATION_TAGS_H
#define INSTRUMENTATION_TAGS_H

#include <array>
#include <type_traits>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <instrumentation/instrumentation_export_.h>

namespace instrumentation {


using tag_value = std::variant<bool, std::int64_t, std::string_view>;
using tag_map = std::unordered_map<std::string_view, tag_value>;

struct tag_entry {
  std::string_view name;
  tag_value value;

  constexpr tag_entry(std::string_view name, bool value) noexcept
  : name(name),
    value(std::in_place_type<bool>, value)
  {}

  template<typename T, typename = std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<bool, T>>>
  constexpr tag_entry(std::string_view name, T value) noexcept
  : name(name),
    value(std::in_place_type<std::int64_t>, value)
  {}

  constexpr tag_entry(std::string_view name, std::string_view value) noexcept
  : name(name),
    value(std::in_place_type<std::string_view>, value)
  {}

  auto apply(tag_map& map) const -> void {
    map[name] = value;
  }
};

struct unbound_tag_entry_ {
  std::string_view name;

  template<typename T>
  constexpr auto operator=(const T& v) const
  noexcept
  -> std::enable_if_t<std::is_constructible_v<tag_entry, const std::string_view&, const T&>, tag_entry> {
    return tag_entry(name, v);
  }
};

template<std::size_t N>
class tags_ {
 public:
  template<
      typename... TagEntries,
      typename = std::enable_if_t<sizeof...(TagEntries) == N>>
  constexpr tags_(TagEntries&&... entries) noexcept
  : values_{{ std::forward<TagEntries>(entries)... }}
  {}

  auto apply(tag_map& map) const
  -> void {
    for (const tag_entry& entry : values_) entry.apply(map);
  }

  template<std::size_t Idx>
  constexpr auto entry() const noexcept
  -> std::enable_if_t<Idx < N, const tag_entry&> {
    return values_[Idx];
  }

  auto as_map() const
  -> tag_map {
    tag_map result;
    apply(result);
    return result;
  }

 private:
  std::array<tag_entry, N> values_;
};

template<>
class tags_<0> {
 public:
  constexpr tags_() noexcept = default;

  auto apply(tag_map& map) const
  noexcept
  -> void {
    /* SKIP */
  }

  auto as_map() const
  -> tag_map {
    return {};
  }
};


template<std::size_t N, std::size_t... Idx>
constexpr auto tags_concat_(const tags_<N>& tags, const tag_entry& entry, std::index_sequence<Idx...>) {
  return tags_<N + 1>(tags.template entry<Idx>()..., entry);
}

template<std::size_t N>
constexpr auto operator|(const tags_<N>& tags, const tag_entry& entry) noexcept
-> tags_<N + 1> {
  if constexpr(N == 0) {
    return tags_<1>(entry);
  } else {
    return tags_concat_(tags, entry, std::make_index_sequence<N>());
  }
}

constexpr auto operator|(const tag_entry& x, const tag_entry& y) noexcept
-> tags_<2> {
  return tags_<2>(x, y);
}

using tags = tags_<0>;

inline namespace literals {
  constexpr auto operator"" _tag(const char* name, std::size_t len) -> unbound_tag_entry_ {
    return unbound_tag_entry_{ std::string_view(name, len) };
  }
}


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_TAGS_H */
