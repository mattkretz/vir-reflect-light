/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2024      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef VIR_FIXED_STRING_H_
#define VIR_FIXED_STRING_H_

#include <concepts>
#include <string_view>

namespace vir
{
  using std::size_t;

  template <size_t N>
    class fixed_string
    {
    public:
      const char data_[N + 1] = {};

      // types
      using value_type = char;
      using pointer = value_type*;
      using const_pointer = const value_type*;
      using reference = value_type&;
      using const_reference = const value_type&;
      using size_type = size_t;
      using difference_type = std::ptrdiff_t;

      // [fixed.string.cons], construction and assignment
      template <std::convertible_to<char>... Chars>
        requires(sizeof...(Chars) == N) and (... and not std::is_pointer_v<Chars>)
        consteval explicit
        fixed_string(Chars... chars) noexcept
        : data_{static_cast<char>(chars)...}
        {}

      template <size_t... Is>
        consteval
        fixed_string(std::index_sequence<Is...>, const char (&txt)[sizeof...(Is) + 1]) noexcept
        : data_{txt[Is]...}
        {}

      consteval
      fixed_string(const char (&txt)[N + 1]) noexcept
      : fixed_string(std::make_index_sequence<N>(), txt)
      {}

      constexpr
      fixed_string(const fixed_string&) noexcept = default;

      constexpr fixed_string&
      operator=(const fixed_string&) noexcept = default;

      // capacity
      static constexpr std::integral_constant<size_type, N> size{};

      static constexpr std::integral_constant<size_type, N> length{};

      static constexpr std::integral_constant<size_type, N> max_size{};

      static constexpr std::bool_constant<N == 0> empty{};

      // element access
      constexpr const_reference
      operator[](size_type pos) const
      { return data_[pos]; }

      constexpr const_reference
      front() const
      { return data_[0]; }

      constexpr const_reference
      back() const
      { return data_[size.value - 1]; }

      // [fixed.string.ops], string operations
      constexpr const_pointer
      c_str() const noexcept
      { return data_; }

      constexpr const_pointer
      data() const noexcept
      { return data_; }

      constexpr std::string_view
      view() const noexcept
      { return {data_, size}; }

      constexpr
      operator std::string_view() const noexcept
      { return {data_, size}; }

      template <size_t N2>
        constexpr friend fixed_string<N + N2>
        operator+(const fixed_string& lhs, const fixed_string<N2>& rhs) noexcept
        {
          return [&]<size_t... Is>(std::index_sequence<Is...>) {
            return fixed_string<N + N2>{(Is < N ? lhs[Is] : rhs[Is - N])...};
          }(std::make_index_sequence<N + N2>());
        }

      constexpr friend fixed_string<N + 1>
      operator+(const fixed_string& lhs, char rhs) noexcept
      {
        return [&]<size_t... Is>(std::index_sequence<Is...>) {
          return fixed_string<N + 1>{(Is < N ? lhs[Is] : rhs)...};
        }(std::make_index_sequence<N + 1>());
      }

      constexpr friend fixed_string<1 + N>
      operator+(const char lhs, const fixed_string& rhs) noexcept
      {
        return [&]<size_t... Is>(std::index_sequence<Is...>) {
          return fixed_string<N + 1>{(Is < 1 ? lhs : rhs[Is - 1])...};
        }(std::make_index_sequence<N + 1>());
      }

      template <size_t N2>
        consteval friend fixed_string<N + N2 - 1>
        operator+(const fixed_string& lhs, const char (&rhs)[N2]) noexcept
        {
          return [&]<size_t... Is>(std::index_sequence<Is...>) {
            return fixed_string<N + N2 - 1>{(Is < N ? lhs[Is] : rhs[Is - N])...};
          }(std::make_index_sequence<N + N2 - 1>());
        }

      template <size_t N1>
        consteval friend fixed_string<N1 + N - 1>
        operator+(const char (&lhs)[N1], const fixed_string& rhs) noexcept
        {
          return [&]<size_t... Is>(std::index_sequence<Is...>) {
            return fixed_string<N1 + N - 1>{(Is < N1 ? lhs[Is] : rhs[Is - N1])...};
          }(std::make_index_sequence<N1 + N - 1>());
        }

      // [fixed.string.comparison], non-member comparison functions
      template <size_t N2>
        friend constexpr bool
        operator==(const fixed_string& lhs, const fixed_string<N2>& rhs)
        { return lhs.view() == rhs.view(); }

      template <size_t N2>
        friend consteval bool
        operator==(const fixed_string& lhs, const char (&rhs)[N2])
        { return lhs.view() == std::string_view(rhs, rhs + N2 - 1); }

      template <size_t N2>
        friend constexpr decltype(auto)
        operator<=>(const fixed_string& lhs, const fixed_string<N2>& rhs)
        { return lhs.view() <=> rhs.view(); }

      template <size_t N2>
        friend consteval decltype(auto)
        operator<=>(const fixed_string& lhs, const char (&rhs)[N2])
        { return lhs.view() <=> std::string_view(rhs, rhs + N2 - 1); }
    };

  // [fixed.string.deduct], deduction guides
  template <std::convertible_to<char>... Rest>
    fixed_string(char, Rest...) -> fixed_string<1 + sizeof...(Rest)>;

  template <size_t N>
    fixed_string(const char (&str)[N]) -> fixed_string<N - 1>;

  template <fixed_string S>
    using fixed_string_type = std::integral_constant<fixed_string<S.size>, S>;

  namespace literals
  {
    template <fixed_string S>
      constexpr auto
      operator""_fs()
      { return S; }

    template <fixed_string S>
      constexpr fixed_string_type<S>
      operator""_ic()
      { return {}; }
  }
}

#endif  // VIR_FIXED_STRING_H_
