#include <instrumentation/tags.h>

namespace instrumentation {


tags::~tags() noexcept {}

auto tags::with(std::string_view name, value_type value)
-> tags& {
  map_[name] = std::move(value);
  return *this;
}


} /* namespace instrumentation */
