/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2024      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef VIR_SIMPLE_TUPLE_H_
#define VIR_SIMPLE_TUPLE_H_

namespace vir
{
  using std::size_t;

  namespace detail
  {
    template <auto X>
      inline constexpr std::integral_constant<std::remove_const_t<decltype(X)>, X> ic {};

#define VIR_SIMPLE_TUPLE_DATA_COMPARES                         \
  friend constexpr bool                                        \
  operator==(tuple_data const&, tuple_data const&) = default;  \
                                                               \
  friend constexpr auto                                        \
  operator<=>(tuple_data const&, tuple_data const&) = default

#define VIR_SIMPLE_TUPLE_DATA_IMPL(i)                          \
  T##i value##i;                                               \
                                                               \
  template <size_t Idx>                                        \
    requires (Idx == Off + i)                                  \
    friend constexpr T##i&&                                    \
    get(tuple_data&& obj) noexcept                             \
    { return static_cast<T##i&&>(obj.value##i); }              \
                                                               \
  template <size_t Idx>                                        \
    requires (Idx == Off + i)                                  \
    friend constexpr T##i&                                     \
    get(tuple_data& obj) noexcept                              \
    { return obj.value##i; }                                   \
                                                               \
  template <size_t Idx>                                        \
    requires (Idx == Off + i)                                  \
    friend constexpr T##i const&                               \
    get(tuple_data const& obj) noexcept                        \
    { return obj.value##i; }                                   \
                                                               \
  template <typename Idx>                                      \
    requires (Idx::value == Off + i)                           \
    friend consteval T##i                                      \
    type_at_impl_(tuple_data const&, Idx)                      \
    { return std::declval<T##i>(); }

    template <int Off, typename... Ts>
      struct tuple_data;

    template <>
      struct tuple_data<0>
      { VIR_SIMPLE_TUPLE_DATA_COMPARES; };

    template <int Off, typename T0>
      struct tuple_data<Off, T0>
      {
        VIR_SIMPLE_TUPLE_DATA_IMPL(0)
        VIR_SIMPLE_TUPLE_DATA_COMPARES;
      };

    template <int Off, typename T0, typename T1>
      struct tuple_data<Off, T0, T1>
      {
        VIR_SIMPLE_TUPLE_DATA_IMPL(0)
        VIR_SIMPLE_TUPLE_DATA_IMPL(1)
        VIR_SIMPLE_TUPLE_DATA_COMPARES;
      };

    template <int Off, typename T0, typename T1, typename T2>
      struct tuple_data<Off, T0, T1, T2>
      {
        VIR_SIMPLE_TUPLE_DATA_IMPL(0)
        VIR_SIMPLE_TUPLE_DATA_IMPL(1)
        VIR_SIMPLE_TUPLE_DATA_IMPL(2)
        VIR_SIMPLE_TUPLE_DATA_COMPARES;
      };

    template <int Off, typename T0, typename T1, typename T2, typename T3>
      struct tuple_data <Off, T0, T1, T2, T3>
      {
        VIR_SIMPLE_TUPLE_DATA_IMPL(0)
        VIR_SIMPLE_TUPLE_DATA_IMPL(1)
        VIR_SIMPLE_TUPLE_DATA_IMPL(2)
        VIR_SIMPLE_TUPLE_DATA_IMPL(3)
        VIR_SIMPLE_TUPLE_DATA_COMPARES;
      };

    template <int Off, typename T0, typename T1, typename T2, typename T3, typename... More>
      requires (sizeof...(More) > 0) and (sizeof...(More) <= 4)
      struct tuple_data<Off, T0, T1, T2, T3, More...>
      : tuple_data<Off, T0, T1, T2, T3>, tuple_data<Off + 4, More...>
      { VIR_SIMPLE_TUPLE_DATA_COMPARES; };

    template <int Off, typename T0, typename T1, typename T2, typename T3,
              typename T4, typename T5, typename T6, typename T7, typename... More>
      requires (sizeof...(More) > 0)
      struct tuple_data<Off, T0, T1, T2, T3, T4, T5, T6, T7, More...>
      : tuple_data<Off, T0, T1, T2, T3>, tuple_data<Off + 4, T4, T5, T6, T7>,
        tuple_data<Off + 8, More...>
      { VIR_SIMPLE_TUPLE_DATA_COMPARES; };

#undef VIR_SIMPLE_TUPLE_DATA_IMPL
#undef VIR_SIMPLE_TUPLE_DATA_COMPARES
  }

  template <typename... Ts>
    class simple_tuple : public detail::tuple_data<0, Ts...>
    {
    public:
      static constexpr auto size = detail::ic<sizeof...(Ts)>;

      static constexpr auto size_sequence = std::make_index_sequence<sizeof...(Ts)>();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
      template <typename... Us>
        requires (sizeof...(Ts) == sizeof...(Us)) and (std::convertible_to<Us&&, Ts> and ...)
        constexpr
        simple_tuple(Us&&... init)
        : detail::tuple_data<0, Ts...> {static_cast<Us&&>(init)...}
        {}
#pragma GCC diagnostic pop

      constexpr
      simple_tuple() = default;

      template <typename Idx>
        constexpr decltype(auto)
        operator[](Idx i) && noexcept
        {
          static_assert(Idx::value >= 0 and Idx::value < sizeof...(Ts));
          return get<i>(static_cast<simple_tuple&&>(*this));
        }

      template <typename Idx>
        constexpr decltype(auto)
        operator[](Idx i) & noexcept
        {
          static_assert(Idx::value >= 0 and Idx::value < sizeof...(Ts));
          return get<i>(*this);
        }

      template <typename Idx>
        constexpr decltype(auto)
        operator[](Idx i) const& noexcept
        {
          static_assert(Idx::value >= 0 and Idx::value < sizeof...(Ts));
          return get<i>(*this);
        }

      template <size_t Idx>
        requires (Idx < sizeof...(Ts))
        using type_at = decltype(type_at_impl_(std::declval<detail::tuple_data<0, Ts...>>(),
                                               detail::ic<Idx>));

      friend constexpr bool
      operator==(simple_tuple const&, simple_tuple const&) = default;

      friend constexpr auto
      operator<=>(simple_tuple const&, simple_tuple const&) = default;

      // concatenation
      template <typename... Us>
        friend constexpr simple_tuple<Ts..., Us...>
        operator+(simple_tuple& lhs, simple_tuple<Us...>& rhs)
        {
          return [&]<size_t... Is, size_t... Js>(std::index_sequence<Is...>,
                                                 std::index_sequence<Js...>) {
            return simple_tuple<Ts..., Us...> {
              get<Is>(lhs)..., get<Js>(rhs)...
            };
          }(lhs.size_sequence, rhs.size_sequence);
        }

      template <typename... Us>
        friend constexpr simple_tuple<Ts..., Us...>
        operator+(simple_tuple const& lhs, simple_tuple<Us...> const& rhs)
        {
          return [&]<size_t... Is, size_t... Js>(std::index_sequence<Is...>,
                                                 std::index_sequence<Js...>) {
            return simple_tuple<Ts..., Us...> {
              get<Is>(lhs)..., get<Js>(rhs)...
            };
          }(lhs.size_sequence, rhs.size_sequence);
        }

      // algorithms
      constexpr void
      for_each(auto&& fun)
      {
        [&]<size_t... Is>(std::index_sequence<Is...>) {
          (fun(get<Is>(*this)), ...);
        }(size_sequence);
      }

      constexpr void
      for_each(auto&& fun) const
      {
        [&]<size_t... Is>(std::index_sequence<Is...>) {
          (fun(get<Is>(*this)), ...);
        }(size_sequence);
      }

      constexpr auto
      for_all(auto&& fun) &
      {
        return [&]<size_t... Is>(std::index_sequence<Is...>) {
          return fun(get<Is>(*this)...);
        }(size_sequence);
      }

      constexpr auto
      for_all(auto&& fun) const&
      {
        return [&]<size_t... Is>(std::index_sequence<Is...>) {
          return fun(get<Is>(*this)...);
        }(size_sequence);
      }

      constexpr auto
      for_all(auto&& fun) &&
      {
        return [&]<size_t... Is>(std::index_sequence<Is...>) {
          return fun(get<Is>(std::move(*this))...);
        }(size_sequence);
      }

      constexpr auto
      transform(auto&& fun) const
      {
        return [&]<size_t... Is>(std::index_sequence<Is...>) {
          return simple_tuple<std::remove_cvref_t<decltype(fun(get<Is>(*this)))>...> {
            fun(get<Is>(*this))...
          };
        }(size_sequence);
      }
    };

  template <typename... Ts>
    simple_tuple(Ts...) -> simple_tuple<Ts...>;

  template <typename... Ts>
    constexpr simple_tuple<Ts...>
    tie(Ts&&... refs) noexcept
    { return simple_tuple<Ts...> {static_cast<Ts&&>(refs)...}; }
}

template <typename... Ts>
  struct std::tuple_size<vir::simple_tuple<Ts...>>
    : std::integral_constant<std::size_t, sizeof...(Ts)>
  {};

template <std::size_t I, typename... Ts>
  struct std::tuple_element<I, vir::simple_tuple<Ts...>>
  { using type = typename vir::simple_tuple<Ts...>::template type_at<I>; };

#endif  // VIR_SIMPLE_TUPLE_H_
