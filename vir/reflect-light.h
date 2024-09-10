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
  0 __VA_OPT__(+ VIR_REFLECT_LIGHT_EXPAND(VIR_REFLECT_LIGHT_COUNT_ARGS_IMPL(__VA_ARGS__)))
#define VIR_REFLECT_LIGHT_COUNT_ARGS_IMPL(x, ...) 1                                                \
  __VA_OPT__(+ VIR_REFLECT_LIGHT_COUNT_ARGS_AGAIN VIR_REFLECT_LIGHT_PARENS(__VA_ARGS__))
#define VIR_REFLECT_LIGHT_COUNT_ARGS_AGAIN() VIR_REFLECT_LIGHT_COUNT_ARGS_IMPL

namespace vir::refl::detail
{
  template <typename T, typename U>
    struct make_dependent
    { using type = U; };

  template <typename T, typename U>
    using make_dependent_t = typename make_dependent<T, U>::type;
}

#define VIR_MAKE_REFLECTABLE(T, ...)                                                               \
public:                                                                                            \
  friend void                                                                                      \
  vir_refl_determine_base_type(...);                                                               \
                                                                                                   \
  template <std::derived_from<T> U>                                                                \
    requires (not std::is_same_v<U, T>)                                                            \
      and std::is_void_v<decltype(vir_refl_determine_base_type(                                    \
                                    std::declval<vir::refl::detail::make_dependent_t<U, T>>()))>   \
    friend T                                                                                       \
    vir_refl_determine_base_type(U const&);                                                        \
                                                                                                   \
  template <std::derived_from<T> U, typename Not>                                                  \
    requires (not std::is_same_v<U, T>) and (not std::derived_from<Not, T>)                        \
      and std::is_void_v<decltype(vir_refl_determine_base_type(                                    \
                                    std::declval<vir::refl::detail::make_dependent_t<U, T>>(),     \
                                    std::declval<Not>()))>                                         \
    friend T                                                                                       \
    vir_refl_determine_base_type(U const&, Not const&);                                            \
                                                                                                   \
  using vir_refl_class_name = vir::fixed_string_type<#T>;                                          \
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
    namespace detail
    {
      template <typename T>
        concept class_type = std::is_class_v<T>;

      //void vir_refl_determine_base_type(...);

      struct None {};

      template <typename T, typename Excluding>
        using find_base
          = decltype(vir_refl_determine_base_type(std::declval<T>(), std::declval<Excluding>()));

      template <typename T, typename Last = None>
        struct base_type_impl
        { using type = void; };

      // if Last is None we're starting the search
      template <class_type T>
        struct base_type_impl<T, None>
        {
          using type
            = typename base_type_impl<T, decltype(vir_refl_determine_base_type(
                                                    std::declval<T>()))>::type;
        };

      // if Last is void => there's no base type (void)
      template <class_type T>
        struct base_type_impl<T, void>
        { using type = void; };

      // otherwise, if find_base<T, Last> is void, Last is the base type
      template <class_type T, class_type Last>
        requires std::derived_from<T, Last> and std::is_void_v<find_base<T, Last>>
        struct base_type_impl<T, Last>
        { using type = Last; };

      // otherwise, find_base<T, Last> is the next Last => recurse
      template <class_type T, class_type Last>
        requires std::derived_from<T, Last> and (not std::is_void_v<find_base<T, Last>>)
        struct base_type_impl<T, Last>
        { using type = typename base_type_impl<T, find_base<T, Last>>::type; };

      template <typename T>
        constexpr typename base_type_impl<T>::type const&
        to_base_type(T const& obj)
        { return obj; }

      template <typename T>
        constexpr typename base_type_impl<T>::type&
        to_base_type(T& obj)
        { return obj; }

      template <auto X>
        inline constexpr std::integral_constant<std::remove_const_t<decltype(X)>, X> ic = {};

      template <auto X>
        inline constexpr auto string_value = X;
    }

    template <typename T>
      inline constexpr auto const& class_name
        = detail::string_value<
            resize(T::vir_refl_class_name::value,
                   detail::ic<T::vir_refl_class_name::value.find_char('<')>)>;

    template <typename T>
      using base_type = typename detail::base_type_impl<T>::type;

    template <typename T>
      constexpr bool is_reflectable = false;

    template <detail::class_type T>
      requires requires { { T::vir_refl_data_member_count } -> std::convertible_to<size_t>; }
      constexpr bool is_reflectable<T> = true;

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
        using Class = std::remove_cvref_t<decltype(obj)>;
        using BaseType = base_type<Class>;
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
