/* SPDX-License-Identifier: GPL-3.0-or-later */
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

#define VIR_TUPLE_DATA_COMPARES                                \
  friend constexpr bool                                        \
  operator==(tuple_data const&, tuple_data const&) = default;  \
                                                               \
  friend constexpr auto                                        \
  operator<=>(tuple_data const&, tuple_data const&) = default

    template <int Off, typename... Ts>
      struct tuple_data;

    template <>
      struct tuple_data<0>
      { VIR_TUPLE_DATA_COMPARES; };

    template <int Off, typename T0>
      struct tuple_data<Off, T0>
      {
        T0 value;

        template <typename Idx>
          requires (Idx::value == Off)
          friend constexpr T0&&
          get(tuple_data&& obj, Idx) noexcept
          { return static_cast<T0&&>(obj.value); }

        template <typename Idx>
          requires (Idx::value == Off)
          friend constexpr T0&
          get(tuple_data& obj, Idx) noexcept
          { return obj.value; }

        template <typename Idx>
          requires (Idx::value == Off)
          friend constexpr T0 const&
          get(tuple_data const& obj, Idx) noexcept
          { return obj.value; }

        template <typename Idx>
          requires (Idx::value == Off)
          friend consteval T0
          type_at_impl(tuple_data const&, Idx)
          { return std::declval<T0>(); }

        VIR_TUPLE_DATA_COMPARES;
      };

    template <int Off, typename T0, typename T1>
      struct tuple_data<Off, T0, T1>
      : tuple_data<Off, T0>, tuple_data<Off + 1, T1>
      { VIR_TUPLE_DATA_COMPARES; };

    template <int Off, typename T0, typename T1, typename T2>
      struct tuple_data<Off, T0, T1, T2>
      : tuple_data<Off, T0>, tuple_data<Off + 1, T1>, tuple_data<Off + 2, T2>
      { VIR_TUPLE_DATA_COMPARES; };

    template <int Off, typename T0, typename T1, typename T2, typename T3>
      struct tuple_data <Off, T0, T1, T2, T3>
      : tuple_data<Off, T0>, tuple_data<Off + 1, T1>,
        tuple_data<Off + 2, T2>, tuple_data<Off + 3, T3>
      { VIR_TUPLE_DATA_COMPARES; };

    template <int Off, typename T0, typename T1, typename T2, typename T3, typename... More>
      requires (sizeof...(More) > 0) and (sizeof...(More) <= 4)
      struct tuple_data<Off, T0, T1, T2, T3, More...>
      : tuple_data<Off, T0>, tuple_data<Off + 1, T1>,
        tuple_data<Off + 2, T2>, tuple_data<Off + 3, T3>,
        tuple_data<Off + 4, More...>
      { VIR_TUPLE_DATA_COMPARES; };

    template <int Off, typename T0, typename T1, typename T2, typename T3,
              typename T4, typename T5, typename T6, typename T7, typename... More>
      requires (sizeof...(More) > 0)
      struct tuple_data<Off, T0, T1, T2, T3, T4, T5, T6, T7, More...>
      : tuple_data<Off, T0>, tuple_data<Off + 1, T1>,
        tuple_data<Off + 2, T2>, tuple_data<Off + 3, T3>,
        tuple_data<Off + 4, T4>, tuple_data<Off + 5, T5>,
        tuple_data<Off + 6, T6>, tuple_data<Off + 7, T7>,
        tuple_data<Off + 8, More...>
      { VIR_TUPLE_DATA_COMPARES; };

#undef VIR_TUPLE_DATA_COMPARES
  }

  template <typename... Ts>
    class simple_tuple : public detail::tuple_data<0, Ts...>
    {
    public:
      static constexpr auto size = detail::ic<sizeof...(Ts)>;

      static constexpr auto size_sequence = std::make_index_sequence<sizeof...(Ts)>();

      template <typename... Us>
        requires (sizeof...(Ts) == sizeof...(Us)) and (std::convertible_to<Us, Ts> and ...)
        constexpr
        simple_tuple(Us&&... init)
        : detail::tuple_data<0, Ts...> {static_cast<Ts>(init)...}
        {}

      template <typename Idx>
        constexpr decltype(auto)
        operator[](Idx i) && noexcept
        {
          static_assert(Idx::value >= 0 and Idx::value < sizeof...(Ts));
          return get(static_cast<simple_tuple&&>(*this), i);
        }

      template <typename Idx>
        constexpr decltype(auto)
        operator[](Idx i) & noexcept
        {
          static_assert(Idx::value >= 0 and Idx::value < sizeof...(Ts));
          return get(*this, i);
        }

      template <typename Idx>
        constexpr decltype(auto)
        operator[](Idx i) const& noexcept
        {
          static_assert(Idx::value >= 0 and Idx::value < sizeof...(Ts));
          return get(*this, i);
        }

      template <size_t Idx>
        requires (Idx < sizeof...(Ts))
        using type_at = decltype(type_at_impl(std::declval<detail::tuple_data<0, Ts...>>(),
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
              get(lhs, detail::ic<Is>)..., get(rhs, detail::ic<Js>)...
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
              get(lhs, detail::ic<Is>)..., get(rhs, detail::ic<Js>)...
            };
          }(lhs.size_sequence, rhs.size_sequence);
        }

      // algorithms
      constexpr void
      for_each(auto&& fun)
      {
        [&]<size_t... Is>(std::index_sequence<Is...>) {
          (fun(get(*this, detail::ic<Is>)), ...);
        }(size_sequence);
      }

      constexpr void
      for_each(auto&& fun) const
      {
        [&]<size_t... Is>(std::index_sequence<Is...>) {
          (fun(get(*this, detail::ic<Is>)), ...);
        }(size_sequence);
      }

      constexpr auto
      for_all(auto&& fun)
      {
        return [&]<size_t... Is>(std::index_sequence<Is...>) {
          return fun(get(*this, detail::ic<Is>)...);
        }(size_sequence);
      }

      constexpr auto
      for_all(auto&& fun) const
      {
        return [&]<size_t... Is>(std::index_sequence<Is...>) {
          return fun(get(*this, detail::ic<Is>)...);
        }(size_sequence);
      }

      constexpr auto
      transform(auto&& fun) const
      {
        return [&]<size_t... Is>(std::index_sequence<Is...>) {
          return simple_tuple<std::remove_cvref_t<decltype(fun(get(*this, detail::ic<Is>)))>...> {
            fun(get(*this, detail::ic<Is>))...
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
#endif  // VIR_SIMPLE_TUPLE_H_
