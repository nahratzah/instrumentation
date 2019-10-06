#ifndef INSTRUMENTATION_MULTI_ENGINE_H
#define INSTRUMENTATION_MULTI_ENGINE_H

#include <instrumentation/engine.h>
#include <initializer_list>
#include <vector>

namespace instrumentation {


auto multi_engine(std::initializer_list<engine> engines) -> engine;

auto multi_engine(std::vector<engine> engines) -> engine;


} /* namespace instrumentation */

#endif /* INSTRUMENTATION_MULTI_ENGINE_H */
