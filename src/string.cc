#include <instrumentation/string.h>
#include <instrumentation/engine.h>

namespace instrumentation {


string_intf::~string_intf() noexcept = default;

void string_intf::set(std::string_view s) noexcept {
  do_set(std::move(s));
}


string::string(std::string_view name)
: string(engine::global().new_string(name))
{}

string::string(std::string_view name, std::initializer_list<std::pair<const std::string, tags::tag_value>> tags)
: string(name, instrumentation::tags(tags))
{}

string::string(std::string_view name, instrumentation::tags tags)
: string(engine::global().new_string(name, std::move(tags)))
{}

void string::operator=(std::string_view s) const noexcept {
  if (impl_) impl_->set(std::move(s));
}


} /* namespace instrumentation */
