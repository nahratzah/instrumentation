#ifndef INSTRUMENTATION_TAGS_H
#define INSTRUMENTATION_TAGS_H

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <instrumentation/instrumentation_export_.h>

namespace instrumentation {


class tags {
 public:
  using value_type = std::variant<bool, std::int64_t, double, std::string_view, std::string>;
  using map_type = std::unordered_map<std::string_view, value_type>;

  instrumentation_local_ tags() noexcept = default;
  instrumentation_export_ ~tags() noexcept;

  instrumentation_local_
  auto with(std::string_view name, bool value)
  -> tags& {
    return with(name, value_type(std::in_place_type<bool>, value));
  }

  instrumentation_local_
  auto with(std::string_view name, std::int64_t value)
  -> tags& {
    return with(name, value_type(std::in_place_type<std::int64_t>, value));
  }

  instrumentation_local_
  auto with(std::string_view name, std::double_t value)
  -> tags& {
    return with(name, value_type(std::in_place_type<double>, value));
  }

  instrumentation_local_
  auto with(std::string_view name, std::string value)
  -> tags& {
    return with(name, value_type(std::in_place_type<std::string>, value));
  }

  instrumentation_local_
  auto with(std::string_view name, std::string_view value)
  -> tags& {
    return with(name, value_type(std::in_place_type<std::string_view>, value));
  }

  auto with(std::string_view name, const char* value) -> tags& = delete;

  instrumentation_export_ auto with(std::string_view name, value_type value) -> tags&;

  instrumentation_local_
  auto get() const
  -> const map_type& {
    return map_;
  }

  instrumentation_local_
  auto operator*() const
  -> const map_type& {
    return map_;
  }

  instrumentation_local_
  auto operator->() const
  -> const map_type* {
    return &map_;
  }

 private:
  map_type map_;
};


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_TAGS_H */
