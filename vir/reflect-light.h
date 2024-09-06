/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2024      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef VIR_REFLECT_LIGHT_H_
#define VIR_REFLECT_LIGHT_H_

#include "fixed_string.h"
#include <tuple>

namespace vir
{
#define VIR_PARENS ()
#define VIR_EXPAND(...) VIR_EXPAND4(VIR_EXPAND4(VIR_EXPAND4(VIR_EXPAND4(__VA_ARGS__))))
#define VIR_EXPAND4(...) VIR_EXPAND3(VIR_EXPAND3(VIR_EXPAND3(VIR_EXPAND3(__VA_ARGS__))))
#define VIR_EXPAND3(...) VIR_EXPAND2(VIR_EXPAND2(VIR_EXPAND2(VIR_EXPAND2(__VA_ARGS__))))
#define VIR_EXPAND2(...) VIR_EXPAND1(VIR_EXPAND1(VIR_EXPAND1(VIR_EXPAND1(__VA_ARGS__))))
#define VIR_EXPAND1(...) __VA_ARGS__
#define VIR_TO_STRINGS(...) __VA_OPT__(VIR_EXPAND(VIR_TO_STRINGS_IMPL(__VA_ARGS__)))
#define VIR_TO_STRINGS_IMPL(x, ...) \
  fixed_string(#x) __VA_OPT__(, VIR_TO_STRINGS_AGAIN VIR_PARENS(__VA_ARGS__))
#define VIR_TO_STRINGS_AGAIN() VIR_TO_STRINGS_IMPL

#define VIR_MAKE_REFLECTABLE(...)                                                                  \
 public:                                                                                           \
  constexpr decltype(auto)                                                                         \
  refl_data_member(auto idx) &                                                                     \
  {                                                                                                \
    if constexpr (std::convertible_to<decltype(idx), std::size_t>)                                 \
      return std::get<idx.value>(std::tie(__VA_ARGS__));                                           \
    else                                                                                           \
      return [&]<std::size_t... Is>(std::index_sequence<Is...>) -> decltype(auto) {                \
        return std::get<(((idx == std::get<Is>(refl_data_member_names)) * Is) + ...)>(             \
                 std::tie(__VA_ARGS__));                                                           \
      }(std::make_index_sequence<refl_data_member_count>());                                       \
  }                                                                                                \
                                                                                                   \
  constexpr decltype(auto)                                                                         \
  refl_data_member(auto idx) const&                                                                \
  {                                                                                                \
    if constexpr (std::convertible_to<decltype(idx), std::size_t>)                                 \
      return std::get<idx.value>(std::tie(__VA_ARGS__));                                           \
    else                                                                                           \
      return [&]<std::size_t... Is>(std::index_sequence<Is...>) -> decltype(auto) {                \
        return std::get<(((idx == std::get<Is>(refl_data_member_names)) * Is) + ...)>(             \
                 std::tie(__VA_ARGS__));                                                           \
      }(std::make_index_sequence<refl_data_member_count>());                                       \
  }                                                                                                \
                                                                                                   \
  static constexpr                                                                                 \
  std::integral_constant<std::size_t, std::tuple_size_v<decltype(std::make_tuple(__VA_ARGS__))>>   \
    refl_data_member_count {};                                                                     \
                                                                                                   \
  static constexpr auto refl_data_member_names = std::tuple{VIR_TO_STRINGS(__VA_ARGS__)}

  namespace _test_
  {
    using namespace vir::literals;
    struct Test
    {
      int a, b, foo;

      VIR_MAKE_REFLECTABLE(a, b, foo);
    };

    template <auto X>
      static constexpr std::integral_constant<std::remove_const_t<decltype(X)>, X>
      ic = {};

    static_assert([] {
      Test t {1, 2, 3};
      if (t.refl_data_member_count != 3)
        return false;
      if (std::get<0>(t.refl_data_member_names) != "a")
        return false;
      if (std::get<1>(t.refl_data_member_names) != "b")
        return false;
      if (std::get<2>(t.refl_data_member_names) != "foo")
        return false;
      if (&t.refl_data_member(ic<0>) != &t.a)
        return false;
      if (&t.refl_data_member(ic<1>) != &t.b)
        return false;
      if (&t.refl_data_member(ic<2>) != &t.foo)
        return false;
      if (&t.refl_data_member("a"_ic) != &t.a)
        return false;
      if (&t.refl_data_member("b"_ic) != &t.b)
        return false;
      if (&t.refl_data_member("foo"_ic) != &t.foo)
        return false;
      t.refl_data_member(ic<1>) = -1;
      if (t.b != -1)
        return false;
      return true;
    }());
  }
}

#endif  // VIR_REFLECT_LIGHT_H_
