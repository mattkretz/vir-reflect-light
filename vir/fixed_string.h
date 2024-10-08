/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2024      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef VIR_FIXED_STRING_H_
#define VIR_FIXED_STRING_H_

#include <concepts>
#include <memory>
#include <string_view>

namespace vir
{
  using std::size_t;

  template <size_t N>
    class fixed_string;

  namespace detail
  {
    template <typename T>
      struct possible_constexpr_string_tmpl_args;

    template <template <typename, size_t> class Str, size_t N>
      requires (sizeof(Str<char, N>) == N + 1)
      struct possible_constexpr_string_tmpl_args<Str<char, N>>
      : std::integral_constant<size_t, N>
      {};

    template <template <size_t> class Str, size_t N>
      requires (sizeof(Str<N>) == N + 1)
      struct possible_constexpr_string_tmpl_args<Str<N>>
      : std::integral_constant<size_t, N>
      {};

    template <template <size_t, typename> class Str, size_t N>
      requires (sizeof(Str<N, char>) == N + 1)
      struct possible_constexpr_string_tmpl_args<Str<N, char>>
      : std::integral_constant<size_t, N>
      {};

    struct empty_string_data
    {};
  }

  template <typename T>
    concept other_fixed_string
      = std::convertible_to<T, std::string_view>
          and requires { detail::possible_constexpr_string_tmpl_args<T>::value; };

  template <size_t N>
    class fixed_string
    {
    public:
      std::conditional_t<N == 0, detail::empty_string_data, char[N == 0 ? 1 : N]> data_;

      // types
      using value_type = char;
      using pointer = value_type*;
      using const_pointer = const value_type*;
      using reference = value_type&;
      using const_reference = const value_type&;
      using iterator = value_type*;
      using const_iterator = const value_type*;  // see fixed.string.iterators
      using size_type = size_t;
      using difference_type = std::ptrdiff_t;

      // internal
      template <size_t... Is>
        constexpr explicit
        fixed_string(std::index_sequence<Is...>, const char *txt) noexcept
        : data_{txt[Is]...}
        { static_assert(sizeof...(Is) == N); }

      // construction and assignment
      constexpr
      fixed_string() = default;

      template <std::convertible_to<char>... Chars>
        requires(sizeof...(Chars) == N) and (... and not std::is_pointer_v<Chars>)
          and (N != 0)
        constexpr explicit
        fixed_string(Chars... chars) noexcept
        : data_{static_cast<char>(chars)...}
        {}

      consteval // implicit conversion from string-literals
      fixed_string(const char (&str)[N + 1]) noexcept
      : fixed_string(std::make_index_sequence<N>(), str)
      {}

      // precondition: std::distance(begin, end) == N
      template <typename It, std::sentinel_for<It> S>
      constexpr
      fixed_string(It begin, S /*end*/) noexcept
      : fixed_string(std::make_index_sequence<N>(), std::to_address(begin))
      {}

      constexpr
      fixed_string(other_fixed_string auto other) noexcept
      : fixed_string(std::make_index_sequence<N>(),
                         static_cast<std::string_view>(other).data())
      {}

      constexpr
      fixed_string(const fixed_string&) noexcept = default;

      constexpr fixed_string&
      operator=(const fixed_string&) noexcept = default;

      // iterator support
      constexpr iterator
      begin() noexcept
      {
        if constexpr (N == 0)
          return nullptr;
        else
          return data_;
      }

      constexpr const_iterator
      begin() const noexcept
      {
        if constexpr (N == 0)
          return nullptr;
        else
          return data_;
      }

      constexpr iterator
      end() noexcept
      { return begin() + N; }

      constexpr const_iterator
      end() const noexcept
      { return begin() + N; }

      constexpr const_iterator
      cbegin() const noexcept
      {
        if constexpr (N == 0)
          return nullptr;
        else
          return data_;
      }

      constexpr const_iterator
      cend() const noexcept
      { return cbegin() + N; }

      // capacity
      static constexpr std::integral_constant<size_type, N> size{};

      static constexpr std::integral_constant<size_type, N> length{};

      static constexpr std::integral_constant<size_type, N> max_size{};

      [[nodiscard]] static constexpr bool
      empty() noexcept
      { return N == 0; }

      // element access
      constexpr reference
      operator[](size_type pos)
      requires (N != 0)
      { return data_[pos]; }

      constexpr const_reference
      operator[](size_type pos) const
      requires (N != 0)
      { return data_[pos]; }

      constexpr reference
      front()
      requires (N != 0)
      { return data_[0]; }

      constexpr const_reference
      front() const
      requires (N != 0)
      { return data_[0]; }

      constexpr reference
      back()
      requires (N != 0)
      { return data_[N - 1]; }

      constexpr const_reference
      back() const
      requires (N != 0)
      { return data_[N - 1]; }

      // [fixed.string.ops], string operations
      constexpr const_pointer
      data() const noexcept
      requires (N != 0)
      { return data_; }

      constexpr std::string_view
      view() const noexcept
      {
        if constexpr (N == 0)
          return {};
        else
          return {data_, N};
      }

      constexpr
      operator std::string_view() const noexcept
      { return view(); }

      template <size_t N2 = N>
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
        if constexpr (N == 0)
          return fixed_string<1>(rhs);
        else
          return [&]<size_t... Is>(std::index_sequence<Is...>) {
            return fixed_string<N + 1>{(Is < N ? lhs[Is] : rhs)...};
          }(std::make_index_sequence<N + 1>());
      }

      constexpr friend fixed_string<1 + N>
      operator+(const char lhs, const fixed_string& rhs) noexcept
      {
        if constexpr (N == 0)
          return fixed_string<1>(lhs);
        else
          return [&]<size_t... Is>(std::index_sequence<Is...>) {
            return fixed_string<N + 1>{(Is < 1 ? lhs : rhs[Is - 1])...};
          }(std::make_index_sequence<N + 1>());
      }

      template <size_t N2>
        constexpr friend fixed_string<N + N2 - 1>
        operator+(const fixed_string& lhs, const char (&rhs)[N2]) noexcept
        {
          if constexpr (N == 0)
            return fixed_string<N2 - 1>(rhs, rhs + N2 - 1);
          else
            return [&]<size_t... Is>(std::index_sequence<Is...>) {
              return fixed_string<N + N2 - 1>{(Is < N ? lhs[Is] : rhs[Is - N])...};
            }(std::make_index_sequence<N + N2 - 1>());
        }

      template <size_t N1>
        constexpr friend fixed_string<N1 + N - 1>
        operator+(const char (&lhs)[N1], const fixed_string& rhs) noexcept
        {
          if constexpr (N == 0)
            return fixed_string<N1 - 1>(lhs, lhs + N1 - 1);
          else
            return [&]<size_t... Is>(std::index_sequence<Is...>) {
              return fixed_string<N1 + N - 1>{(Is < N1 - 1 ? lhs[Is] : rhs[Is - N1 + 1])...};
            }(std::make_index_sequence<N1 + N - 1>());
        }

      constexpr size_t
      find_char(char c) const noexcept
      {
        if constexpr (N == 0)
          return 0;
        else
          {
            for (size_t i = 0; i < N; ++i)
              {
                if (data_[i] == c)
                  return i;
              }
            return N;
          }
      }

      template <typename NewSize>
        friend consteval fixed_string<NewSize::value>
        resize(const fixed_string& old, NewSize) noexcept
        {
          static_assert(NewSize::value <= N);
          return fixed_string<NewSize::value>(
                   std::make_index_sequence<NewSize::value>(), old.data_);
        }

      template <typename Offset,
                typename NewSize = std::integral_constant<size_t, N - Offset::value>>
        friend consteval fixed_string<NewSize::value>
        substring(const fixed_string& old, Offset, NewSize = {}) noexcept
        {
          static_assert(Offset::value + NewSize::value <= N);
          static_assert(Offset::value >= 0);
          static_assert(NewSize::value >= 0);
          return fixed_string<NewSize::value>(
                   std::make_index_sequence<NewSize::value>(), old.data_ + Offset::value);
        }

      // [fixed.string.comparison], non-member comparison functions
      friend constexpr bool
      operator==(const fixed_string& lhs, const fixed_string& rhs)
      {
        if constexpr (N == 0)
          return true;
        else
          return lhs.view() == rhs.view();
      }

      template <size_t N2>
        requires (N2 != N)
        friend constexpr bool
        operator==(const fixed_string&, const fixed_string<N2>&)
        { return false; }

      template <size_t N2>
        requires (N2 != N + 1)
        friend constexpr bool
        operator==(const fixed_string&, const char (&)[N2])
        { return false; }

      friend constexpr decltype(auto)
      operator<=>(const fixed_string& lhs, const fixed_string& rhs)
      { return lhs.view() <=> rhs.view(); }

      template <size_t N2>
        requires (N2 != N + 1)
        friend constexpr decltype(auto)
        operator<=>(const fixed_string& lhs, const fixed_string<N2>& rhs)
        { return lhs.view() <=> rhs.view(); }

      template <size_t N2>
        requires (N2 != N + 1)
        friend constexpr decltype(auto)
        operator<=>(const fixed_string& lhs, const char (&rhs)[N2])
        { return lhs.view() <=> std::string_view(rhs, rhs + N2 - 1); }
    };

  template <fixed_string S, typename T = std::remove_const_t<decltype(S)>>
    class constexpr_string;

  // fixed_string deduction guides
  template <std::convertible_to<char>... Rest>
    fixed_string(char, Rest...) -> fixed_string<1 + sizeof...(Rest)>;

  template <size_t N>
    fixed_string(const char (&str)[N]) -> fixed_string<N - 1>;

  template <other_fixed_string T>
    fixed_string(T) -> fixed_string<detail::possible_constexpr_string_tmpl_args<T>::value>;

  template <auto S>
    fixed_string(constexpr_string<S>) -> fixed_string<S.size>;

  // The T parameter exists solely for enabling lookup of fixed_string operators (ADL).
  template <fixed_string S, typename T>
    class constexpr_string
    {
    public:
      static constexpr auto value = S;

      constexpr
      operator const T&() const
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
      data() const noexcept
      { return S.data_; }

      consteval std::string_view
      view() const noexcept
      { return {S.data_, size}; }

      consteval
      operator std::string_view() const noexcept
      { return view(); }

      template <fixed_string S2>
        consteval friend constexpr_string<S + S2>
        operator+(constexpr_string, constexpr_string<S2>) noexcept
        { return {}; }

      template <typename rhs>
        requires std::same_as<decltype(rhs::value), const char>
        consteval friend constexpr_string<S + rhs::value>
        operator+(constexpr_string, rhs) noexcept
        { return {}; }

      template <typename lhs>
        requires std::same_as<decltype(lhs::value), const char>
        consteval friend constexpr_string<lhs::value + S>
        operator+(lhs, constexpr_string) noexcept
        { return {}; }

      template <typename c>
        requires std::same_as<decltype(c::value), const char>
        consteval std::integral_constant<size_t, S.find_char(c::value)>
        find_char(c) const noexcept
        { return {}; }

      template <typename NewSize>
        consteval constexpr_string<resize(S, NewSize())>
        resize(NewSize) const noexcept
        { return {}; }

      template <typename Offset,
                typename NewSize = std::integral_constant<size_t, size - Offset::value>>
        consteval constexpr_string<substring(S, Offset(), NewSize())>
        substring(Offset, NewSize = {}) const noexcept
        { return {}; }
    };

  namespace detail
  {
    template <std::integral auto N>
      consteval auto
      fixed_string_from_number()
      {
        constexpr int buf_len = [] {
          auto x = N;
          int len = x < 0 ? 1 : 0;
          while (x != 0)
            {
              ++len;
              x /= 10;
            }
          return len;
        }();
        char buffer[buf_len + 1] = {};
        auto x = N;
        constexpr bool negative = N < 0;
        int i = buf_len;
        while (x != 0)
          {
            buffer[--i] = '0' + (x < 0 ? -1 : 1) * (x % 10);
            x /= 10;
          }
        if (negative)
          buffer[--i] = '-';
        if (i != 0)
          throw i;
        return fixed_string<buf_len>(buffer);
      }
  }

  template <std::integral auto N>
    inline constexpr auto fixed_string_from_number = detail::fixed_string_from_number<N>();

  template <std::integral auto N>
    requires (N >= 0 and N < 10)
    inline constexpr auto fixed_string_from_number<N> = fixed_string<1>('0' + N);

  template <std::integral auto N>
    using constexpr_string_from_number = constexpr_string<fixed_string_from_number<N>>;

  template <std::integral auto N>
    inline constexpr constexpr_string_from_number<N> constexpr_string_from_number_v {};

  namespace literals
  {
    template <fixed_string S>
      constexpr constexpr_string<S>
      operator""_fs()
      { return {}; }
  }
}

#endif  // VIR_FIXED_STRING_H_
