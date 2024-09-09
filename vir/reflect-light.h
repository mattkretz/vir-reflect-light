/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2024      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef VIR_REFLECT_LIGHT_H_
#define VIR_REFLECT_LIGHT_H_

#include "fixed_string.h"
#include <tuple>

// recursive macro implementation inspired by https://www.scs.stanford.edu/~dm/blog/va-opt.html

#define VIR_REFLECT_LIGHT_PARENS ()
#define VIR_REFLECT_LIGHT_EXPAND(...) VIR_REFLECT_LIGHT_EXPAND3(VIR_REFLECT_LIGHT_EXPAND3(         \
                                        VIR_REFLECT_LIGHT_EXPAND3(VIR_REFLECT_LIGHT_EXPAND3(       \
                                          __VA_ARGS__))))
#define VIR_REFLECT_LIGHT_EXPAND3(...) VIR_REFLECT_LIGHT_EXPAND2(VIR_REFLECT_LIGHT_EXPAND2(        \
                                         VIR_REFLECT_LIGHT_EXPAND2(VIR_REFLECT_LIGHT_EXPAND2(      \
                                           __VA_ARGS__))))
#define VIR_REFLECT_LIGHT_EXPAND2(...) VIR_REFLECT_LIGHT_EXPAND1(VIR_REFLECT_LIGHT_EXPAND1(        \
                                         VIR_REFLECT_LIGHT_EXPAND1(VIR_REFLECT_LIGHT_EXPAND1(      \
                                           __VA_ARGS__))))
#define VIR_REFLECT_LIGHT_EXPAND1(...) __VA_ARGS__

#define VIR_REFLECT_LIGHT_TO_STRINGS(...)                                                          \
  __VA_OPT__(VIR_REFLECT_LIGHT_EXPAND(VIR_REFLECT_LIGHT_TO_STRINGS_IMPL(__VA_ARGS__)))
#define VIR_REFLECT_LIGHT_TO_STRINGS_IMPL(x, ...) ::vir::fixed_string(#x)                          \
  __VA_OPT__(, VIR_REFLECT_LIGHT_TO_STRINGS_AGAIN VIR_REFLECT_LIGHT_PARENS(__VA_ARGS__))
#define VIR_REFLECT_LIGHT_TO_STRINGS_AGAIN() VIR_REFLECT_LIGHT_TO_STRINGS_IMPL

#define VIR_REFLECT_LIGHT_COUNT_ARGS(...)                                                          \
  __VA_OPT__(VIR_REFLECT_LIGHT_EXPAND(VIR_REFLECT_LIGHT_COUNT_ARGS_IMPL(__VA_ARGS__)))
#define VIR_REFLECT_LIGHT_COUNT_ARGS_IMPL(x, ...) 1                                                \
  __VA_OPT__(+ VIR_REFLECT_LIGHT_COUNT_ARGS_AGAIN VIR_REFLECT_LIGHT_PARENS(__VA_ARGS__))
#define VIR_REFLECT_LIGHT_COUNT_ARGS_AGAIN() VIR_REFLECT_LIGHT_COUNT_ARGS_IMPL

#define VIR_MAKE_REFLECTABLE(T, ...)                                                               \
 public:                                                                                           \
  template <typename U>                                                                            \
    requires (not std::is_same_v<U, T>)                                                            \
    friend T                                                                                       \
    vir_refl_determine_base_type(U const&);                                                        \
                                                                                                   \
  friend void                                                                                      \
  vir_refl_determine_base_type(...);                                                               \
                                                                                                   \
  constexpr decltype(auto)                                                                         \
  vir_refl_members_as_tuple() &                                                                    \
  { return std::tie(__VA_ARGS__); }                                                                \
                                                                                                   \
  constexpr decltype(auto)                                                                         \
  vir_refl_members_as_tuple() const&                                                               \
  { return std::tie(__VA_ARGS__); }                                                                \
                                                                                                   \
  static constexpr std::integral_constant<std::size_t, VIR_REFLECT_LIGHT_COUNT_ARGS(__VA_ARGS__)>  \
    vir_refl_data_member_count {};                                                                 \
                                                                                                   \
  static constexpr auto vir_refl_data_member_names                                                 \
    = std::tuple{VIR_REFLECT_LIGHT_TO_STRINGS(__VA_ARGS__)}

namespace vir
{
  namespace refl
  {
    template <typename T>
      requires (not std::is_class_v<T>)
      void vir_refl_determine_base_type(T const&);

    template <typename T>
      using base_type = decltype(vir_refl_determine_base_type(std::declval<T>()));

    namespace detail
    {
      template <typename T>
        constexpr base_type<T> const&
        to_base_type(T const& obj)
        { return obj; }

      template <typename T>
        constexpr base_type<T>&
        to_base_type(T& obj)
        { return obj; }
    }

    template <typename T>
      constexpr size_t data_member_count = 0;

    template <typename T>
      requires std::is_class_v<T> and std::is_void_v<base_type<T>>
      constexpr size_t data_member_count<T> = T::vir_refl_data_member_count;

    template <typename T>
      requires std::is_class_v<T> and (not std::is_void_v<base_type<T>>)
      constexpr size_t data_member_count<T>
        = T::vir_refl_data_member_count + data_member_count<base_type<T>>;

    template <typename T, size_t Idx>
      constexpr auto data_member_name = [] {
        static_assert(Idx < data_member_count<T>);
        return fixed_string("Error");
      }();

    template <typename T, size_t Idx>
      requires (Idx < data_member_count<base_type<T>>)
      constexpr auto data_member_name<T, Idx> = data_member_name<base_type<T>, Idx>;

    template <typename T, size_t Idx>
      requires (Idx >= data_member_count<base_type<T>>) and (Idx < data_member_count<T>)
      constexpr auto data_member_name<T, Idx>
        = std::get<Idx - data_member_count<base_type<T>>>(T::vir_refl_data_member_names);

    template <size_t Idx>
      constexpr decltype(auto)
      data_member(auto&& obj)
      {
        using BaseType = decltype(vir_refl_determine_base_type(obj));
        constexpr size_t base_size = data_member_count<BaseType>;
        if constexpr (Idx < base_size)
          return data_member<Idx>(detail::to_base_type(obj));
        else
          return std::get<Idx - base_size>(obj.vir_refl_members_as_tuple());
      }

    template <fixed_string Name>
      constexpr decltype(auto)
      data_member(auto&& obj)
      {
        using Class = std::remove_cvref_t<decltype(obj)>;
        return [&]<size_t... Is>(std::index_sequence<Is...>) -> decltype(auto) {
          constexpr size_t idx = ((Name == data_member_name<Class, Is> ? Is : 0) + ...);
          return data_member<idx>(obj);
        }(std::make_index_sequence<data_member_count<Class>>());
      }
  }
}

#endif  // VIR_REFLECT_LIGHT_H_
