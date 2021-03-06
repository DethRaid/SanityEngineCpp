#ifndef RX_CORE_TRAITS_IS_CALLABLE_H
#define RX_CORE_TRAITS_IS_CALLABLE_H
#include "rx/core/traits/is_same.h"

#include "rx/core/utility/nat.h"
#include "rx/core/utility/declval.h"

namespace Rx::traits {

namespace detail {
  template<typename T, typename... Ts>
  struct is_callable {
    template<typename U>
    static auto test(U* _data)
      -> decltype((*_data)(Utility::declval<Ts>()...), void(), int());
    
    template<typename>
    static Utility::Nat test(...);

    static constexpr const auto value{!traits::is_same<decltype(test<T>(nullptr)), Utility::Nat>};
  };
} // namespace detail

template<typename T, typename... Ts>
static constexpr const auto is_callable{detail::is_callable<T, Ts...>::value};

} // namespace rx::traits

#endif // RX_CORE_TRAITS_IS_CALLABLE_H