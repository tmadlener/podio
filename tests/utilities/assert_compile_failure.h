#ifndef PODIO_TESTS_UTILITIES_ASSERTCOMPILEFAILURE_H // NOLINT(llvm-header-guard): folder structure not suitable
#define PODIO_TESTS_UTILITIES_ASSERTCOMPILEFAILURE_H // NOLINT(llvm-header-guard): folder structure not suitable

#include <algorithm>

#if __has_include("type_traits/experimental")
  #include <type_traits/experimental>
namespace test_utils {
using is_detected = std::experimental::is_detected;
}
#else
namespace test_utils {
namespace detail {
  template <class Default, class AlwaysVoid, template <class...> class Op, class... Args>
  struct detector {
    using value_t = std::false_type;
    using type = Default;
  };

  template <class Default, template <class...> class Op, class... Args>
  struct detector<Default, std::void_t<Op<Args...>>, Op, Args...> {
    using value_t = std::true_type;
    using type = Op<Args...>;
  };

  struct nonesuch {
    ~nonesuch() = delete;
    nonesuch(nonesuch const&) = delete;
    void operator=(nonesuch const&) = delete;
  };
} // namespace detail

template <template <class...> class Op, class... Args>
using is_detected = typename detail::detector<detail::nonesuch, void, Op, Args...>::value_t;

template <template <class...> class Op, class... Args>
using detected_t = typename detail::detector<detail::nonesuch, void, Op, Args...>::type;

template <typename It, typename ElemT>
using is_fillable_t = decltype(std::fill(std::declval<It>(), std::declval<It>(), std::declval<ElemT>()));

template <typename It, typename E>
constexpr static bool fill_compiles = is_detected<is_fillable_t, It, E>::value;

} // namespace test_utils
#endif // __has_include

#endif
