#ifndef PODIO_BENCHMARK_UTIL_H__
#define PODIO_BENCHMARK_UTIL_H__

#include <chrono>
#include <functional>
#include <utility>
#include <type_traits>

namespace podio::benchmark {
using ClockT = std::chrono::high_resolution_clock;

/**
 * Run a function and record the duration. Return the result and the duration
 * in a pair unless the return type is void, then only return the duration.
 */
template<class Func, typename ...Args>
decltype(auto) run_timed(Func&& func, Args&&... args) {
  const auto start = ClockT::now();

  // Have to special case here because std::invoke doesn't handle void and
  // non-void return types on equal footing (at least for now).
  // A bit of code duplication is not really avoidable in this case
  if constexpr(std::is_same_v<std::invoke_result_t<Func, Args...>, void>) {
    std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
    const auto end = ClockT::now();
    return end - start;
  } else {
    const auto retval = std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
    const auto end = ClockT::now();
    return std::make_pair(retval, end - start);
  }
}

/**
 * Run a member function and record the duration. Return the result and the
 * duration in a pair if the return type of the member function is non-void,
 * otherwise only the duration.
 *
 * Simply delegates to run_timed while taking care of passing all the parameters
 * in the right order.
 */
template<class Obj, typename MemberFunc, typename ...Args>
decltype(auto) run_member_timed(Obj&& obj, MemberFunc&& func, Args&&... args) {
  return run_timed(std::forward<MemberFunc>(func),
                   obj,
                   std::forward<Args>(args)...);
}

} // namespace podio::benchmark

#endif
