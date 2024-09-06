/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2024      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef VIR_REFLECT_LIGHT_H_
#define VIR_REFLECT_LIGHT_H_

#include "fixed_string.h"
#include <tuple>

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

#define VIR_COUNT_ARGS(...) __VA_OPT__(VIR_EXPAND(VIR_COUNT_ARGS_IMPL(__VA_ARGS__)))
#define VIR_COUNT_ARGS_IMPL(x, ...) 1 __VA_OPT__(+ VIR_COUNT_ARGS_AGAIN VIR_PARENS(__VA_ARGS__))
#define VIR_COUNT_ARGS_AGAIN() VIR_COUNT_ARGS_IMPL

#define VIR_MAKE_REFLECTABLE(...)                                                                  \
 public:                                                                                           \
  constexpr decltype(auto)                                                                         \
  vir_refl_members_as_tuple() &                                                                    \
  { return std::tie(__VA_ARGS__); }                                                                \
                                                                                                   \
  constexpr decltype(auto)                                                                         \
  vir_refl_members_as_tuple() const&                                                               \
  { return std::tie(__VA_ARGS__); }                                                                \
                                                                                                   \
  static constexpr std::integral_constant<std::size_t, VIR_COUNT_ARGS(__VA_ARGS__)>                \
    vir_refl_data_member_count {};                                                                 \
                                                                                                   \
  static constexpr auto vir_refl_data_member_names = std::tuple{VIR_TO_STRINGS(__VA_ARGS__)}

namespace vir
{
  namespace refl
  {
    template <size_t Idx>
      constexpr decltype(auto)
      data_member(auto&& obj)
      { return std::get<Idx>(obj.vir_refl_members_as_tuple()); }

    template <fixed_string Name>
      constexpr decltype(auto)
      data_member(auto&& obj)
      {
        using Class = std::remove_cvref_t<decltype(obj)>;
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) -> decltype(auto) {
          return std::get<(((Name == std::get<Is>(Class::vir_refl_data_member_names)) * Is) + ...)>(
                   obj.vir_refl_members_as_tuple());
        }(std::make_index_sequence<Class::vir_refl_data_member_count>());
      }

    template <typename T>
      constexpr auto data_member_count = T::vir_refl_data_member_count;

    template <typename T, size_t Idx>
      constexpr auto data_member_name = std::get<Idx>(T::vir_refl_data_member_names);
  }

  namespace _test_
  {
    struct Test
    {
      int a, b, foo;

      VIR_MAKE_REFLECTABLE(a, b, foo);
    };

    static_assert([] {
      Test t {1, 2, 3};
      if (refl::data_member_count<Test>  != 3)
        return false;
      if (refl::data_member_name<Test, 0> != "a")
        return false;
      if (refl::data_member_name<Test, 1> != "b")
        return false;
      if (refl::data_member_name<Test, 2> != "foo")
        return false;
      if (&refl::data_member<0>(t) != &t.a)
        return false;
      if (&refl::data_member<1>(t) != &t.b)
        return false;
      if (&refl::data_member<2>(t) != &t.foo)
        return false;
      if (&refl::data_member<"a">(t) != &t.a)
        return false;
      if (&refl::data_member<"b">(t) != &t.b)
        return false;
      if (&refl::data_member<"foo">(t) != &t.foo)
        return false;
      refl::data_member<"b">(t) = -1;
      if (t.b != -1)
        return false;
      return true;
    }());
  }
}

#endif  // VIR_REFLECT_LIGHT_H_
