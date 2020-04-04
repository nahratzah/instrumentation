#ifndef INSTRUMENTATION_FWD_H
#define INSTRUMENTATION_FWD_H

namespace instrumentation {


class engine;
class collector;

class counter;
template<typename... LabelTypes> class counter_vector;


} /* namespace instrumentation */

namespace instrumentation::detail {


class counter_impl;


} /* namespace instrumentation::detail */

#endif /* INSTRUMENTATION_FWD_H */
