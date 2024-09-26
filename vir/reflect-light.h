/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2024      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef VIR_REFLECT_LIGHT_H_
#define VIR_REFLECT_LIGHT_H_

#include "fixed_string.h"
#include "simple_tuple.h"

#include <array>
#include <string>

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
#define VIR_REFLECT_LIGHT_TO_STRINGS_IMPL(x, ...) ::vir::fixed_string<#x>()                        \
  __VA_OPT__(, VIR_REFLECT_LIGHT_TO_STRINGS_AGAIN VIR_REFLECT_LIGHT_PARENS(__VA_ARGS__))
#define VIR_REFLECT_LIGHT_TO_STRINGS_AGAIN() VIR_REFLECT_LIGHT_TO_STRINGS_IMPL

#define VIR_REFLECT_LIGHT_COUNT_ARGS(...)                                                          \
  0 __VA_OPT__(+ VIR_REFLECT_LIGHT_EXPAND(VIR_REFLECT_LIGHT_COUNT_ARGS_IMPL(__VA_ARGS__)))
#define VIR_REFLECT_LIGHT_COUNT_ARGS_IMPL(x, ...) 1                                                \
  __VA_OPT__(+ VIR_REFLECT_LIGHT_COUNT_ARGS_AGAIN VIR_REFLECT_LIGHT_PARENS(__VA_ARGS__))
#define VIR_REFLECT_LIGHT_COUNT_ARGS_AGAIN() VIR_REFLECT_LIGHT_COUNT_ARGS_IMPL

#define VIR_REFLECT_LIGHT_DECLTYPES(...)                                                           \
  __VA_OPT__(VIR_REFLECT_LIGHT_EXPAND(VIR_REFLECT_LIGHT_DECLTYPES_IMPL(__VA_ARGS__)))
#define VIR_REFLECT_LIGHT_DECLTYPES_IMPL(x, ...) decltype(x)                                       \
  __VA_OPT__(, VIR_REFLECT_LIGHT_DECLTYPES_AGAIN VIR_REFLECT_LIGHT_PARENS(__VA_ARGS__))
#define VIR_REFLECT_LIGHT_DECLTYPES_AGAIN() VIR_REFLECT_LIGHT_DECLTYPES_IMPL

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
  vir_refl_determine_base_type(T const&, ...)                                                      \
  {}                                                                                               \
                                                                                                   \
  template <std::derived_from<T> U>                                                                \
    requires (not std::is_same_v<U, T>)                                                            \
      and std::is_void_v<decltype(vir_refl_determine_base_type(                                    \
                                    std::declval<vir::refl::detail::make_dependent_t<U, T>>(), 0))>\
    friend T                                                                                       \
    vir_refl_determine_base_type(U const&, int) { return std::declval<T>(); }                      \
                                                                                                   \
  template <std::derived_from<T> U, typename Not>                                                  \
    requires (not std::is_same_v<U, T>) and (not std::derived_from<Not, T>)                        \
      and std::is_void_v<decltype(vir_refl_determine_base_type(                                    \
                                    std::declval<vir::refl::detail::make_dependent_t<U, T>>(),     \
                                    std::declval<Not>()))>                                         \
    friend T                                                                                       \
    vir_refl_determine_base_type(U const&, Not const&) { return std::declval<T>(); }               \
                                                                                                   \
  using vir_refl_class_name = vir::fixed_string<#T>;                                               \
                                                                                                   \
  constexpr auto                                                                                   \
  vir_refl_members_as_tuple() &                                                                    \
  { return vir::tie(__VA_ARGS__); }                                                                \
                                                                                                   \
  constexpr auto                                                                                   \
  vir_refl_members_as_tuple() const&                                                               \
  { return vir::tie(__VA_ARGS__); }                                                                \
                                                                                                   \
  using vir_refl_data_member_types = vir::simple_tuple<VIR_REFLECT_LIGHT_DECLTYPES(__VA_ARGS__)>;  \
                                                                                                   \
  static constexpr std::integral_constant<std::size_t, VIR_REFLECT_LIGHT_COUNT_ARGS(__VA_ARGS__)>  \
    vir_refl_data_member_count {};                                                                 \
                                                                                                   \
  static constexpr auto vir_refl_data_member_names                                                 \
    = vir::simple_tuple{VIR_REFLECT_LIGHT_TO_STRINGS(__VA_ARGS__)}

namespace vir
{
  namespace refl
  {
    namespace detail
    {
      template <typename T>
        concept class_type = std::is_class_v<T>;

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
                                                    std::declval<T>(), 0))>::type;
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

      template <typename T>
        consteval auto
        type_to_string(T*)
        {
#ifdef __GNUC__
          constexpr auto fun = __PRETTY_FUNCTION__;
          constexpr size_t fun_size = sizeof(__PRETTY_FUNCTION__) - 1;
          constexpr auto offset_size
            = [&] () -> std::pair<size_t, size_t> {
              size_t offset = 0;
              for (; offset < fun_size and fun[offset] != '='; ++offset)
                ;
              if (offset + 2 >= fun_size or offset < 20 or fun[offset + 1] != ' '
                    or fun[offset - 2] != 'T')
                return {0, fun_size};
              offset += 2; // skip over '= '
              size_t size = 0;
              for (; offset + size < fun_size and fun[offset + size] != ']'; ++size)
                ;
              return {offset, size};
            }();
#elif defined _MSC_VER
          constexpr auto fun = __FUNCSIG__;
          constexpr size_t fun_size = sizeof(__FUNCSIG__) - 1;
          constexpr auto offset_size
            = [&] () -> std::pair<size_t, size_t> {
              size_t offset = 0;
              for (; offset < fun_size and fun[offset] != '<'; ++offset)
                ;
              if (offset + 2 >= fun_size or offset < 20 or fun[offset - 1] != 'g')
                return {0, fun_size};
              offset += 1; // skip over '<'
              // remove 'struct ', 'union ', 'class ', or 'enum ' prefix.
              if (std::string_view(fun + offset, 7) == "struct ")
                offset += 7;
              else if (std::string_view(fun + offset, 6) == "class ")
                offset += 6;
              else if (std::string_view(fun + offset, 6) == "union ")
                offset += 6;
              else if (std::string_view(fun + offset, 5) == "enum ")
                offset += 5;
              size_t size = 0;
              for (; offset + size < fun_size and fun[offset + size] != '('; ++size)
                ;
              return {offset, size - 1};
            }();
#else
#error "Compiler not supported."
#endif
          constexpr size_t offset = offset_size.first;
          constexpr size_t size = offset_size.second;
          static_assert(offset < fun_size);
          static_assert(size <= fun_size);
          static_assert(offset + size <= fun_size);
          return fixed_string_arg<size>(fun + offset);
        }

      template <auto X>
        consteval auto
        nttp_to_string()
        {
#ifdef __GNUC__
          constexpr auto fun = __PRETTY_FUNCTION__;
          constexpr size_t fun_size = sizeof(__PRETTY_FUNCTION__) - 1;
          constexpr auto offset_size
            = [&] () -> std::pair<size_t, size_t> {
              size_t offset = 0;
              for (; offset < fun_size and fun[offset] != '='; ++offset)
                ;
              if (offset + 2 >= fun_size or offset < 20 or fun[offset + 1] != ' '
                    or fun[offset - 2] != 'X')
                return {0, fun_size};
              offset += 2; // skip over '= '
              size_t size = 0;
              for (; offset + size < fun_size and fun[offset + size] != ']'; ++size)
                ;
              return {offset, size};
            }();
#elif defined _MSC_VER
          constexpr auto fun = __FUNCSIG__;
          constexpr size_t fun_size = sizeof(__FUNCSIG__) - 1;
          constexpr auto offset_size
            = [&] () -> std::pair<size_t, size_t> {
              size_t offset = 0;
              for (; offset < fun_size and fun[offset] != '<'; ++offset)
                ;
              if (offset + 2 >= fun_size or offset < 20 or fun[offset - 1] != 'g')
                return {0, fun_size};
              offset += 1; // skip over '<'
              size_t size = 0;
              for (; offset + size < fun_size and fun[offset + size] != '('; ++size)
                ;
              return {offset, size - 1};
            }();
#else
#error "Compiler not supported."
#endif
          constexpr size_t offset = offset_size.first;
          constexpr size_t size = offset_size.second;
          static_assert(offset < fun_size);
          static_assert(size <= fun_size);
          static_assert(offset + size <= fun_size);
          return fixed_string_arg<size>(fun + offset);
        }
    }

    template <typename T>
      concept reflectable = std::is_class_v<std::remove_cvref_t<T>> and requires {
        { std::remove_cvref_t<T>::vir_refl_data_member_count } -> std::convertible_to<size_t>;
      };

    template <typename T>
      inline constexpr auto type_name
        = fixed_string<detail::type_to_string(static_cast<T*>(nullptr))>();

    template <auto T>
      requires std::is_enum_v<decltype(T)>
      inline constexpr auto enum_name
        = fixed_string<detail::nttp_to_string<T>()>();

#define VIR_SPECIALIZE_TYPE_NAME(T)                                                                \
    template <>                                                                                    \
      inline constexpr auto type_name<T> = vir::fixed_string<#T> {}

    VIR_SPECIALIZE_TYPE_NAME(bool);
    VIR_SPECIALIZE_TYPE_NAME(char);
    VIR_SPECIALIZE_TYPE_NAME(wchar_t);
    VIR_SPECIALIZE_TYPE_NAME(char8_t);
    VIR_SPECIALIZE_TYPE_NAME(char16_t);
    VIR_SPECIALIZE_TYPE_NAME(char32_t);
    VIR_SPECIALIZE_TYPE_NAME(signed char);
    VIR_SPECIALIZE_TYPE_NAME(unsigned char);
    VIR_SPECIALIZE_TYPE_NAME(short);
    VIR_SPECIALIZE_TYPE_NAME(unsigned short);
    VIR_SPECIALIZE_TYPE_NAME(int);
    VIR_SPECIALIZE_TYPE_NAME(unsigned int);
    VIR_SPECIALIZE_TYPE_NAME(long);
    VIR_SPECIALIZE_TYPE_NAME(unsigned long);
    VIR_SPECIALIZE_TYPE_NAME(long long);
    VIR_SPECIALIZE_TYPE_NAME(unsigned long long);
    VIR_SPECIALIZE_TYPE_NAME(float);
    VIR_SPECIALIZE_TYPE_NAME(double);
    VIR_SPECIALIZE_TYPE_NAME(long double);
    VIR_SPECIALIZE_TYPE_NAME(std::string);

#undef VIR_SPECIALIZE_TYPE_NAME

    template <typename T>
      inline constexpr auto class_name
#ifdef _MSC_VER
        = resize(type_name<T>.value, detail::ic<type_name<T>.value.find_char('<')>);
#else
        = type_name<T>.resize(type_name<T>.find_char(detail::ic<'<'>));
#endif

    template <typename T>
      using base_type = typename detail::base_type_impl<T>::type;

    template <typename T>
      constexpr size_t data_member_count = 0;

    template <reflectable T>
      requires std::is_void_v<base_type<T>>
      constexpr size_t data_member_count<T> = T::vir_refl_data_member_count;

    template <reflectable T>
      requires (not std::is_void_v<base_type<T>>)
      constexpr size_t data_member_count<T>
        = T::vir_refl_data_member_count + data_member_count<base_type<T>>;

    template <typename T, size_t Idx>
      constexpr auto data_member_name = [] {
        static_assert(Idx < data_member_count<T>);
        return vir::fixed_string<"Error">();
      }();

    template <reflectable T, size_t Idx>
      requires (Idx < data_member_count<base_type<T>>)
      constexpr auto data_member_name<T, Idx> = data_member_name<base_type<T>, Idx>;

    template <reflectable T, size_t Idx>
      requires (Idx >= data_member_count<base_type<T>>) and (Idx < data_member_count<T>)
      constexpr auto data_member_name<T, Idx>
        = T::vir_refl_data_member_names[detail::ic<Idx - data_member_count<base_type<T>>>];

    template <reflectable T, fixed_string_arg Name>
      constexpr auto data_member_index
        = detail::ic<[]<size_t... Is>(std::index_sequence<Is...>) {
                      return ((Name == data_member_name<T, Is>.value ? Is : 0) + ...);
                    }(std::make_index_sequence<data_member_count<T>>())>;

    template <size_t Idx>
      constexpr decltype(auto)
      data_member(reflectable auto&& obj)
      {
        using Class = std::remove_cvref_t<decltype(obj)>;
        using BaseType = base_type<Class>;
        constexpr size_t base_size = data_member_count<BaseType>;
        if constexpr (Idx < base_size)
          return data_member<Idx>(detail::to_base_type(obj));
        else
          return obj.vir_refl_members_as_tuple()[detail::ic<Idx - base_size>];
      }

    template <fixed_string_arg Name>
      constexpr decltype(auto)
      data_member(reflectable auto&& obj)
      { return data_member<data_member_index<std::remove_cvref_t<decltype(obj)>, Name>>(obj); }

    constexpr decltype(auto)
    all_data_members(reflectable auto&& obj)
    {
      using B = base_type<std::remove_cvref_t<decltype(obj)>>;
      if constexpr (std::is_void_v<B>)
        return obj.vir_refl_members_as_tuple();
      else
        return all_data_members(static_cast<B&>(obj)) + obj.vir_refl_members_as_tuple();
    }

    namespace detail
    {
      template <size_t N>
        struct data_member_id
        : vir::fixed_string_arg<N>
        {
          static constexpr bool is_name = N != 0;

          const size_t index;

          consteval
          data_member_id(const char* txt) requires (N != 0)
            : fixed_string_arg<N>(txt), index(-1)
          {}

          consteval
          data_member_id(std::convertible_to<size_t> auto idx) requires (N == 0)
            : fixed_string_arg<0>(), index(idx)
          {}

          consteval vir::fixed_string_arg<N> const&
          string() const
          { return *this; }
        };

      template <size_t N>
        data_member_id(const char (&str)[N]) -> data_member_id<N - 1>;

      template <std::convertible_to<size_t> T>
        data_member_id(T) -> data_member_id<0>;

      template <typename T, data_member_id Idx>
        struct data_member_type_impl
        : data_member_type_impl<T, data_member_index<T, Idx.string()>>
        {};

      template <typename T, data_member_id Idx>
        requires (not Idx.is_name) and (Idx.index >= data_member_count<base_type<T>>)
        struct data_member_type_impl<T, Idx>
        {
          using type = typename T::vir_refl_data_member_types::template type_at<
                         Idx.index - data_member_count<base_type<T>>>;
        };

      template <typename T, data_member_id Idx>
        requires (not Idx.is_name) and (Idx.index < data_member_count<base_type<T>>)
        struct data_member_type_impl<T, Idx>
        { using type = typename data_member_type_impl<base_type<T>, Idx.index>::type; };
    }

    template <reflectable T, detail::data_member_id Id>
      using data_member_type = typename detail::data_member_type_impl<T, Id>::type;

    template <reflectable T, template <typename, size_t> class Pred>
      constexpr std::array find_data_members = []<size_t... Is>(std::index_sequence<Is...>) {
        constexpr size_t matches = (Pred<T, Is>::value + ...);
        constexpr std::array results = {(Pred<T, Is>::value ? Is : -1)...};
        std::array<size_t, matches> r = {};
        size_t i = 0;
        for (size_t idx : results)
          if (idx != size_t(-1))
            r[i++] = idx;
        return r;
      }(std::make_index_sequence<data_member_count<T>>());

    template <reflectable T, template <typename> class Pred>
      constexpr std::array find_data_members_by_type = []<size_t... Is>(std::index_sequence<Is...>) {
        constexpr size_t matches = (Pred<data_member_type<T, Is>>::value + ...);
        constexpr std::array results = {(Pred<data_member_type<T, Is>>::value ? Is : -1)...};
        std::array<size_t, matches> r = {};
        size_t i = 0;
        for (size_t idx : results)
          if (idx != size_t(-1))
            r[i++] = idx;
        return r;
      }(std::make_index_sequence<data_member_count<T>>());

    template <reflectable T, std::array Idxs = std::array<size_t, 0> {}>
      using data_member_types = decltype([]<size_t... Is>(std::index_sequence<Is...>)
                                           -> vir::simple_tuple<data_member_type<T, Idxs[Is]>...>
      { return {}; }(std::make_index_sequence<Idxs.size()>()));
  }
}

#endif  // VIR_REFLECT_LIGHT_H_
