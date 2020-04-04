#ifndef INSTRUMENTATION_FWD_H
#define INSTRUMENTATION_FWD_H

namespace instrumentation {


class engine;
class collector;

class counter;
template<typename... LabelTypes> class counter_vector;
class gauge;
template<typename... LabelTypes> class gauge_vector;
class string;
template<typename... LabelTypes> class string_vector;
class timing;
template<typename... LabelTypes> class timing_vector;


} /* namespace instrumentation */

namespace instrumentation::detail {


class counter_impl;
class gauge_impl;
class string_impl;
class timing_impl;


} /* namespace instrumentation::detail */

#endif /* INSTRUMENTATION_FWD_H */
