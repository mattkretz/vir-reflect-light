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
    class fixed_string_arg;

  namespace detail
  {
    // This places a string in .rodata without ever duplicating it in the final binary. It is used
    // to guard against dangling pointers from fixed_string::c_str()/data()/view().
    template <fixed_string_arg X>
      inline constexpr auto string_storage = X;
  }

  template <size_t N>
    class fixed_string_arg
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
        fixed_string_arg(Chars... chars) noexcept
        : data_{static_cast<char>(chars)...}
        {}

      template <size_t... Is>
        consteval
        fixed_string_arg(std::index_sequence<Is...>, const char *txt) noexcept
        : data_{txt[Is]...}
        { static_assert(sizeof...(Is) == N); }

      consteval
      fixed_string_arg(const char *txt) noexcept
      : fixed_string_arg(std::make_index_sequence<N>(), txt)
      {}

      constexpr
      fixed_string_arg(const fixed_string_arg&) noexcept = default;

      constexpr fixed_string_arg&
      operator=(const fixed_string_arg&) noexcept = default;

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
      { return data_[N - 1]; }

      // [fixed.string.ops], string operations
      constexpr const_pointer
      c_str() const noexcept
      { return data_; }

      constexpr const_pointer
      data() const noexcept
      { return data_; }

      constexpr std::string_view
      view() const noexcept
      { return {data_, N}; }

      constexpr
      operator std::string_view() const noexcept
      { return {data_, N}; }

      template <size_t N2>
        constexpr friend fixed_string_arg<N + N2>
        operator+(const fixed_string_arg& lhs, const fixed_string_arg<N2>& rhs) noexcept
        {
          return [&]<size_t... Is>(std::index_sequence<Is...>) {
            return fixed_string_arg<N + N2>{(Is < N ? lhs[Is] : rhs[Is - N])...};
          }(std::make_index_sequence<N + N2>());
        }

      constexpr friend fixed_string_arg<N + 1>
      operator+(const fixed_string_arg& lhs, char rhs) noexcept
      {
        return [&]<size_t... Is>(std::index_sequence<Is...>) {
          return fixed_string_arg<N + 1>{(Is < N ? lhs[Is] : rhs)...};
        }(std::make_index_sequence<N + 1>());
      }

      constexpr friend fixed_string_arg<1 + N>
      operator+(const char lhs, const fixed_string_arg& rhs) noexcept
      {
        return [&]<size_t... Is>(std::index_sequence<Is...>) {
          return fixed_string_arg<N + 1>{(Is < 1 ? lhs : rhs[Is - 1])...};
        }(std::make_index_sequence<N + 1>());
      }

      template <size_t N2>
        consteval friend fixed_string_arg<N + N2 - 1>
        operator+(const fixed_string_arg& lhs, const char (&rhs)[N2]) noexcept
        {
          return [&]<size_t... Is>(std::index_sequence<Is...>) {
            return fixed_string_arg<N + N2 - 1>{(Is < N ? lhs[Is] : rhs[Is - N])...};
          }(std::make_index_sequence<N + N2 - 1>());
        }

      template <size_t N1>
        consteval friend fixed_string_arg<N1 + N - 1>
        operator+(const char (&lhs)[N1], const fixed_string_arg& rhs) noexcept
        {
          return [&]<size_t... Is>(std::index_sequence<Is...>) {
            return fixed_string_arg<N1 + N - 1>{(Is < N1 ? lhs[Is] : rhs[Is - N1])...};
          }(std::make_index_sequence<N1 + N - 1>());
        }

      constexpr size_t
      find_char(char c) const noexcept
      {
        for (size_t i = 0; i < N; ++i)
          {
            if (data_[i] == c)
              return i;
          }
        return N;
      }

      template <typename NewSize>
        friend consteval fixed_string_arg<NewSize::value>
        resize(const fixed_string_arg& old, NewSize) noexcept
        {
          static_assert(NewSize::value <= N);
          return fixed_string_arg<NewSize::value>(
                   std::make_index_sequence<NewSize::value>(), old.data_);
        }

      template <typename Offset,
                typename NewSize = std::integral_constant<size_t, N - Offset::value>>
        friend consteval fixed_string_arg<NewSize::value>
        substring(const fixed_string_arg& old, Offset, NewSize = {}) noexcept
        {
          static_assert(Offset::value + NewSize::value <= N);
          static_assert(Offset::value >= 0);
          static_assert(NewSize::value >= 0);
          return fixed_string_arg<NewSize::value>(
                   std::make_index_sequence<NewSize::value>(), old.data_ + Offset::value);
        }

      // [fixed.string.comparison], non-member comparison functions
      template <size_t N2>
        friend constexpr bool
        operator==(const fixed_string_arg& lhs, const fixed_string_arg<N2>& rhs)
        { return lhs.view() == rhs.view(); }

      template <size_t N2>
        friend constexpr bool
        operator==(const fixed_string_arg& lhs, const char (&rhs)[N2])
        { return lhs.view() == std::string_view(rhs, rhs + N2 - 1); }

      template <size_t N2>
        friend constexpr decltype(auto)
        operator<=>(const fixed_string_arg& lhs, const fixed_string_arg<N2>& rhs)
        { return lhs.view() <=> rhs.view(); }

      template <size_t N2>
        friend constexpr decltype(auto)
        operator<=>(const fixed_string_arg& lhs, const char (&rhs)[N2])
        { return lhs.view() <=> std::string_view(rhs, rhs + N2 - 1); }
    };

  // [fixed.string.deduct], deduction guides
  template <std::convertible_to<char>... Rest>
    fixed_string_arg(char, Rest...) -> fixed_string_arg<1 + sizeof...(Rest)>;

  template <size_t N>
    fixed_string_arg(const char (&str)[N]) -> fixed_string_arg<N - 1>;

  template <fixed_string_arg S>
    class fixed_string
    {
    public:
      static constexpr auto value = S;

      constexpr
      operator decltype(S)() const
      { return S; }

      // types
      using value_type = char;
      using pointer = value_type*;
      using const_pointer = const value_type*;
      using reference = value_type&;
      using const_reference = const value_type&;
      using size_type = size_t;
      using difference_type = std::ptrdiff_t;

      // capacity
      static constexpr auto size = S.size;

      static constexpr auto length = S.length;

      static constexpr auto max_size = S.max_size;

      static constexpr auto empty = S.empty;

      // element access
      consteval const_reference
      operator[](size_type pos) const
      { return S[pos]; }

      consteval const_reference
      front() const
      { return S.front(); }

      consteval const_reference
      back() const
      { return S.back(); }

      // string operations
      consteval const_pointer
      c_str() const noexcept
      { return detail::string_storage<S>.data_; }

      consteval const_pointer
      data() const noexcept
      { return detail::string_storage<S>.data_; }

      consteval std::string_view
      view() const noexcept
      { return {detail::string_storage<S>.data_, size}; }

      consteval
      operator std::string_view() const noexcept
      { return view(); }

      template <fixed_string_arg S2>
        consteval friend fixed_string<S + S2>
        operator+(fixed_string, fixed_string<S2>) noexcept
        { return {}; }

      template <typename rhs>
        requires std::same_as<decltype(rhs::value), const char>
        consteval friend fixed_string<S + rhs::value>
        operator+(fixed_string, rhs) noexcept
        { return {}; }

      template <typename lhs>
        requires std::same_as<decltype(lhs::value), const char>
        consteval friend fixed_string<lhs::value + S>
        operator+(lhs, fixed_string) noexcept
        { return {}; }

      template <typename c>
        requires std::same_as<decltype(c::value), const char>
        consteval std::integral_constant<size_t, S.find_char(c::value)>
        find_char(c) const noexcept
        { return {}; }

      template <typename NewSize>
        consteval fixed_string<resize(S, NewSize())>
        resize(NewSize) const noexcept
        { return {}; }

      template <typename Offset,
                typename NewSize = std::integral_constant<size_t, size - Offset::value>>
        consteval fixed_string<substring(S, Offset(), NewSize())>
        substring(Offset, NewSize = {}) const noexcept
        { return {}; }

      // non-member comparison functions
      template <fixed_string_arg S2>
        friend constexpr bool
        operator==(fixed_string, fixed_string<S2>)
        { return S == S2; }

      template <size_t N2>
        friend constexpr bool
        operator==(fixed_string, const char (&rhs)[N2])
        { return S == rhs; }

      template <fixed_string_arg S2>
        friend constexpr decltype(auto)
        operator<=>(fixed_string, fixed_string<S2>)
        { return S <=> S2; }

      template <size_t N2>
        friend constexpr decltype(auto)
        operator<=>(fixed_string, const char (&rhs)[N2])
        { return S <=> rhs; }
    };

  namespace literals
  {
    template <fixed_string_arg S>
      constexpr fixed_string<S>
      operator""_fs()
      { return {}; }
  }
}

#endif  // VIR_FIXED_STRING_H_
