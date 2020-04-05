#ifndef PRINT_H
#define PRINT_H

#include <iosfwd>
#include <vector>

namespace std {


template<typename Char, typename Traits, typename T, typename Alloc>
auto operator<<(std::basic_ostream<Char, Traits>& out, const std::vector<T, Alloc>& v)
-> std::basic_ostream<Char, Traits>& {
  if (v.empty()) return out << "[]";

  out << "[" << v.front();
  for (auto iter = std::next(v.begin()), end = v.end();
      iter != end;
      ++iter)
    out << ", " << *iter;
  return out << "]";
}


} /* namespace std */

#endif /* PRINT_H */
