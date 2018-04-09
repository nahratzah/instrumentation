#include <instrumentation/tags.h>

namespace instrumentation {


using namespace std::string_view_literals;

const std::string_view tags::tls_entry_key = "thread"sv;

tags::~tags() noexcept {}

auto tags::with(std::string_view name, value_type value)
-> tags& {
  map_[name] = std::move(value);
  return *this;
}


} /* namespace instrumentation */
