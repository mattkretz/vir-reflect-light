/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2024      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#include <vir/reflect-light.h>
#include <vir/simple_tuple.h>
#include <utility>
#include <vector>
#include <complex>
#include <string>

[[maybe_unused]] constexpr auto i0 = vir::refl::detail::ic<0>;
[[maybe_unused]] constexpr auto i1 = vir::refl::detail::ic<1>;
[[maybe_unused]] constexpr auto i2 = vir::refl::detail::ic<2>;
[[maybe_unused]] constexpr auto i3 = vir::refl::detail::ic<3>;
[[maybe_unused]] constexpr auto i4 = vir::refl::detail::ic<4>;
/*
[[maybe_unused]] constexpr auto i5 = vir::refl::detail::ic<5>;
[[maybe_unused]] constexpr auto i6 = vir::refl::detail::ic<6>;
[[maybe_unused]] constexpr auto i7 = vir::refl::detail::ic<7>;
[[maybe_unused]] constexpr auto i8 = vir::refl::detail::ic<8>;
[[maybe_unused]] constexpr auto i9 = vir::refl::detail::ic<9>;
[[maybe_unused]] constexpr auto i10 = vir::refl::detail::ic<10>;
[[maybe_unused]] constexpr auto i11 = vir::refl::detail::ic<11>;
[[maybe_unused]] constexpr auto i12 = vir::refl::detail::ic<12>;
[[maybe_unused]] constexpr auto i13 = vir::refl::detail::ic<13>;
[[maybe_unused]] constexpr auto i14 = vir::refl::detail::ic<14>;
[[maybe_unused]] constexpr auto i15 = vir::refl::detail::ic<15>;
*/

// ==============================================
// ================ simple_tuple ================
// ==============================================

static_assert(vir::simple_tuple {1, 2} != vir::simple_tuple {2, 3});
static_assert(vir::simple_tuple {1, 2}[std::integral_constant<int, 0>()] == 1);
static_assert(std::same_as<decltype(vir::simple_tuple {1, 2}[i0]), int&&>);
static_assert(vir::simple_tuple {} == vir::simple_tuple {});
static_assert(vir::simple_tuple {} <= vir::simple_tuple {});
static_assert(std::tuple_size_v<vir::simple_tuple<int, float, char>> == 3);
static_assert(std::same_as<std::tuple_element_t<0, vir::simple_tuple<int, float, char>>, int>);
static_assert(std::same_as<std::tuple_element_t<1, vir::simple_tuple<int, float, char>>, float>);
static_assert(std::same_as<std::tuple_element_t<2, vir::simple_tuple<int, float, char>>, char>);

static_assert([] {
  vir::simple_tuple t3{1, 2, 3};
  static_assert(std::same_as<decltype(t3[i0]), int&>);
  static_assert(std::same_as<decltype(std::as_const(t3)[i0]), int const&>);
  {
    auto [x1, x2, x3] = t3;
    if (not (x1 == 1 and x2 == 2 and x3 == 3))
      return false;
  }
  static_assert(&get<0>(t3) == &t3[i0]);
  static_assert(&get<1>(t3) == &t3[i1]);
  static_assert(&get<2>(t3) == &t3[i2]);
  int a = 0, b = 1, c = 2, d = 3;
  auto t4 = vir::tie(a, b, c, d);
  if (not(t4[i0] == 0 and t4[i1] == 1) and t4[i2] == 2 and t4[i3] == 3)
    return false;
  a = 4;
  if (t4[i0] != 4)
    return false;
  if (&t4[i0] != &a)
    return false;

  vir::simple_tuple t3p1 = t3.transform([](auto x) { return x + 1; });
  if (t3p1 != vir::simple_tuple {2, 3, 4})
    return false;

  auto sum = t3.for_all([](auto... values) {
               return (... + values);
             });
  if (sum != 6)
    return false;

  sum = 0;
  t3.for_all([&](auto... values) {
    sum = (... + values);
  });
  if (sum != 6)
    return false;

  auto t6 = t3 + t3;
  auto t9 = t3 + t6;
  if (t9[i0] != t9[i3])
    return false;
  if (t9 != t6 + t3)
    return false;

  auto t7 = t4 + vir::tie(a, t6, c);
  if (&t7[i0] != &t7[i4])
    return false;

  return true;
}());

namespace simple_tuple_test
{
  struct MoveOnly
  {
    MoveOnly() = default;
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly(MoveOnly&&) = default;
  };

  constexpr auto
  f()
  {
    vir::simple_tuple<MoveOnly, MoveOnly> x {};
    vir::simple_tuple<MoveOnly, MoveOnly> y = std::move(x);
    vir::simple_tuple<MoveOnly, MoveOnly> z
      = std::move(y).for_all([](auto&&... args) {
          return vir::simple_tuple(std::move(args)...);
        });
    return z;
  }
}
// ==============================================
// ================ fixed_string ================
// ==============================================

static_assert(vir::fixed_string_arg("") == "");
static_assert(vir::fixed_string_arg("text") == "text");
static_assert(vir::fixed_string_arg("text") <= "text");
static_assert(vir::fixed_string_arg("text") >= "text");
static_assert(vir::fixed_string_arg("text") != "txet");
static_assert(vir::fixed_string_arg("text") <  "txet");
static_assert(vir::fixed_string_arg("text") <= "txet");

static_assert(""     == vir::fixed_string_arg(""));
static_assert("text" == vir::fixed_string_arg("text"));
static_assert("text" <= vir::fixed_string_arg("text"));
static_assert("text" >= vir::fixed_string_arg("text"));
static_assert("txet" != vir::fixed_string_arg("text"));
static_assert("txet" >  vir::fixed_string_arg("text"));
static_assert("txet" >= vir::fixed_string_arg("text"));

static_assert(vir::fixed_string<"">() == "");
static_assert(vir::fixed_string<"text">() == "text");
static_assert(vir::fixed_string<"text">() <= "text");
static_assert(vir::fixed_string<"text">() >= "text");
static_assert(vir::fixed_string<"text">() != "txet");
static_assert(vir::fixed_string<"text">() <  "txet");
static_assert(vir::fixed_string<"text">() <= "txet");

static_assert("" == vir::fixed_string<"">());
static_assert("text" == vir::fixed_string<"text">());
static_assert("text" <= vir::fixed_string<"text">());
static_assert("text" >= vir::fixed_string<"text">());
static_assert("txet" != vir::fixed_string<"text">());
static_assert("txet" >  vir::fixed_string<"text">());
static_assert("txet" >= vir::fixed_string<"text">());

static_assert(vir::fixed_string_from_number_v<0> == "0");
static_assert(vir::fixed_string_from_number_v<7> == "7");
static_assert(vir::fixed_string_from_number_v<9> == "9");
static_assert(vir::fixed_string_from_number_v<10> == "10");
static_assert(vir::fixed_string_from_number_v<-1> == "-1");
static_assert(vir::fixed_string_from_number_v<123> == "123");
static_assert(vir::fixed_string_from_number_v<(1u<<31)> == "2147483648");
static_assert(vir::fixed_string_from_number_v<int(1u<<31)> == "-2147483648");

static_assert("ab" + vir::fixed_string<"cd">() + "ef" == "abcdef");
static_assert('a' + vir::fixed_string<"cd">() + 'f' == "acdf");

static_assert(std::constructible_from<std::string, vir::fixed_string<"foo">>);

constexpr auto
f()
{
  const vir::fixed_string<"Hello World!"> as_type;
  //constexpr auto as_arg = as_type + ' ' + as_type;
  constexpr auto and_now = "4× " + as_type + " How do you do?";// + as_arg;
  return vir::fixed_string<and_now>();
}

// ==============================================
// =================  vir::refl =================
// ==============================================

static_assert(vir::refl::type_name<int> == vir::fixed_string<"int">());
static_assert(vir::refl::type_name<float> == "float");
static_assert(vir::refl::type_name<std::string> == "std::string");
static_assert(vir::refl::type_name<std::array<int, 4>> == "std::array<int, 4>");
static_assert(vir::refl::type_name<std::vector<int>> == "std::vector<int>");
static_assert(vir::refl::type_name<std::complex<float>> == "std::complex<float>");

namespace ns0
{
  template <typename T, auto X>
    struct NotReflected
    {};

  template <typename>
    class Foo
    {};

  template <typename>
    union Bar
    { int x; float y; };

  enum Enum
  { A, B, C };

  enum class EnumClass
  { Foo, Bar };

  static_assert(vir::refl::class_name<NotReflected<int, 5>> == "ns0::NotReflected");
  static_assert(vir::refl::class_name<Foo<int>> == "ns0::Foo");
  static_assert(vir::refl::class_name<Bar<float>> == "ns0::Bar");
  static_assert(vir::refl::class_name<Enum> == "ns0::Enum");
  static_assert(vir::refl::class_name<EnumClass> == "ns0::EnumClass");
  static_assert(vir::refl::type_name<NotReflected<int, 5>> == "ns0::NotReflected<int, 5>");
  static_assert(vir::refl::type_name<Foo<int>> == "ns0::Foo<int>");
  static_assert(vir::refl::type_name<Bar<float>> == "ns0::Bar<float>");
  static_assert(vir::refl::type_name<Enum> == "ns0::Enum");
  static_assert(vir::refl::type_name<EnumClass> == "ns0::EnumClass");

  static_assert(vir::refl::enum_name<A> == "ns0::A");
  static_assert(vir::refl::enum_name<B> == "ns0::B");
  static_assert(vir::refl::enum_name<C> == "ns0::C");
  static_assert(vir::refl::enum_name<EnumClass::Foo> == "ns0::EnumClass::Foo");

  static_assert(vir::refl::nttp_name<1> == "1" or vir::refl::nttp_name<1> == "0x1");
  static_assert(vir::refl::nttp_name<1u> == "1" or vir::refl::nttp_name<1u> == "1U"
                  or vir::refl::nttp_name<1u> == "0x1");
  static_assert(vir::refl::nttp_name<A> == vir::refl::enum_name<A>);
}

struct Test
{
  int a, b, foo;

  VIR_MAKE_REFLECTABLE(Test, a, b, foo);
};

static_assert(std::same_as<vir::refl::base_type<Test>, void>);
static_assert(vir::refl::data_member_count<Test> == 3);
static_assert(vir::refl::data_member_name<Test, 0> == "a");
static_assert(vir::refl::data_member_name<Test, 1> == "b");
static_assert(vir::refl::data_member_name<Test, 2> == "foo");
static_assert(vir::refl::class_name<Test> == "Test");
static_assert(std::same_as<vir::refl::data_member_type<Test, 0>, int>);
static_assert(std::same_as<vir::refl::data_member_type<Test, 1>, int>);
static_assert(std::same_as<vir::refl::data_member_type<Test, 2>, int>);
static_assert(std::same_as<vir::refl::data_member_type<Test, "a">, int>);
static_assert(std::same_as<vir::refl::data_member_type<Test, "b">, int>);
static_assert(std::same_as<vir::refl::data_member_type<Test, "foo">, int>);

static_assert([] {
  Test t {1, 2, 3};
  if (&vir::refl::data_member<0>(t) != &t.a)
    return false;
  if (&vir::refl::data_member<1>(t) != &t.b)
    return false;
  if (&vir::refl::data_member<2>(t) != &t.foo)
    return false;
  if (&vir::refl::data_member<"a">(t) != &t.a)
    return false;
  if (&vir::refl::data_member<"b">(t) != &t.b)
    return false;
  if (&vir::refl::data_member<"foo">(t) != &t.foo)
    return false;
  vir::refl::data_member<"b">(t) = -1;
  if (t.b != -1)
    return false;
  return true;
}());

int
test0(Test& t)
{ return vir::refl::data_member<"foo">(t); }

auto
test1()
{ return vir::refl::data_member_name<Test, 1>; }

struct Derived : Test
{
  float in;
  double out;

  VIR_MAKE_REFLECTABLE(Derived, in, out);
};

static_assert(vir::refl::reflectable<Derived>);
static_assert(std::same_as<vir::refl::base_type<Derived>, Test>);
static_assert(vir::refl::data_member_count<Derived> == 5);
static_assert(vir::refl::data_member_name<Derived, 0> == "a");
static_assert(vir::refl::data_member_name<Derived, 1> == "b");
static_assert(vir::refl::data_member_name<Derived, 2> == "foo");
static_assert(vir::refl::data_member_name<Derived, 3> == "in");
static_assert(vir::refl::data_member_name<Derived, 4> == "out");
static_assert(vir::refl::class_name<Derived> == "Derived");
static_assert(vir::refl::class_name<vir::refl::base_type<Derived>> == "Test");
static_assert(std::same_as<vir::refl::data_member_type<Derived, 0>, int>);
static_assert(std::same_as<vir::refl::data_member_type<Derived, 1>, int>);
static_assert(std::same_as<vir::refl::data_member_type<Derived, 2>, int>);
static_assert(std::same_as<vir::refl::data_member_type<Derived, 3>, float>);
static_assert(std::same_as<vir::refl::data_member_type<Derived, 4>, double>);
static_assert(std::same_as<vir::refl::data_member_type<Derived, "a">, int>);
static_assert(std::same_as<vir::refl::data_member_type<Derived, "b">, int>);
static_assert(std::same_as<vir::refl::data_member_type<Derived, "foo">, int>);
static_assert(std::same_as<vir::refl::data_member_type<Derived, "in">, float>);
static_assert(std::same_as<vir::refl::data_member_type<Derived, "out">, double>);

static_assert([] {
  Derived t {{1, 2, 3}, 1.1f, 2.2};
  if (&vir::refl::data_member<0>(t) != &t.a)
    return false;
  if (&vir::refl::data_member<1>(t) != &t.b)
    return false;
  if (&vir::refl::data_member<2>(t) != &t.foo)
    return false;
  if (&vir::refl::data_member<3>(t) != &t.in)
    return false;
  if (&vir::refl::data_member<4>(t) != &t.out)
    return false;
  if (&vir::refl::data_member<"a">(t) != &t.a)
    return false;
  if (&vir::refl::data_member<"b">(t) != &t.b)
    return false;
  if (&vir::refl::data_member<"foo">(t) != &t.foo)
    return false;
  if (&vir::refl::data_member<"in">(t) != &t.in)
    return false;
  if (&vir::refl::data_member<"out">(t) != &t.out)
    return false;
  vir::refl::data_member<"in">(t) *= -1;
  if (t.in != -1.1f)
    return false;

  if (&vir::refl::all_data_members(t)[i0] != &t.a)
    return false;
  if (&vir::refl::all_data_members(t)[i1] != &t.b)
    return false;
  if (&vir::refl::all_data_members(t)[i2] != &t.foo)
    return false;
  if (&vir::refl::all_data_members(t)[i3] != &t.in)
    return false;
  if (&vir::refl::all_data_members(t)[i4] != &t.out)
    return false;

  return true;
}());

struct Further : Derived
{
  char c;
  VIR_MAKE_REFLECTABLE(Further, c);
};

static_assert(vir::refl::reflectable<Further>);
static_assert(std::same_as<vir::refl::base_type<Further>, Derived>);

struct AndAnother : Further
{
  // static data members are also supported
  inline static int baz = 1;

  VIR_MAKE_REFLECTABLE(AndAnother, baz);
};

static_assert(vir::refl::reflectable<AndAnother>);
static_assert(std::same_as<vir::refl::base_type<AndAnother>, Further>);

template <typename T, size_t Idx>
using only_floats = std::is_same<vir::refl::data_member_type<T, Idx>, float>;

template <typename T, size_t Idx>
using only_ints = std::is_same<vir::refl::data_member_type<T, Idx>, int>;

template <typename T, size_t Idx>
using name_is_out = std::bool_constant<vir::refl::data_member_name<T, Idx> == "out">;

static_assert(vir::refl::find_data_members<AndAnother, only_floats> == std::array<size_t, 1>{3});
static_assert(vir::refl::find_data_members<AndAnother, only_ints>
                == std::array<size_t, 4>{0, 1, 2, 6});
static_assert(vir::refl::find_data_members<AndAnother, name_is_out> == std::array<size_t, 1>{4});
static_assert(vir::refl::find_data_members_by_type<AndAnother, std::is_floating_point>
                == std::array<size_t, 2>{3, 4});

static_assert([] {
  AndAnother x;
  auto& value = vir::refl::data_member<"baz">(x);
  return &value == &AndAnother::baz;
}());

#if __clang__ < 18
#define ARRAY std::array
#else
#define ARRAY
#endif
static_assert(std::same_as<vir::refl::data_member_types<AndAnother, ARRAY{0, 1}>,
                           vir::simple_tuple<int, int>>);
static_assert(std::same_as<vir::refl::data_member_types<AndAnother, ARRAY{2, 5, 4}>,
                           vir::simple_tuple<int, char, double>>);
static_assert(std::same_as<vir::refl::data_member_types<AndAnother>,
                           vir::simple_tuple<int, int, int, float, double, char, int>>);
#undef ARRAY

static_assert([] {
  size_t sum = 0;
  vir::refl::for_each_data_member_index<AndAnother>([&sum](auto idx) {
    sum += idx;
  });
  return sum;
}() == 0 + 1 + 2 + 3 + 4 + 5 + 6);

namespace ns
{
  template <typename T>
  struct Type
  {
    int blah;
    VIR_MAKE_REFLECTABLE(ns::Type<T>, blah);
  };

  struct Type2 : Type<Type2>
  {
    VIR_MAKE_REFLECTABLE(ns::Type2);
  };
}

struct Type3 : ns::Type<Type3>
{
  float x, y, z;
  VIR_MAKE_REFLECTABLE(Type3, x, y, z);
};

static_assert(vir::refl::reflectable<ns::Type<int>>);
static_assert(std::same_as<vir::refl::base_type<ns::Type<int>>, void>);
static_assert(vir::refl::data_member_count<ns::Type<int>> == 1);
static_assert(vir::refl::type_name<ns::Type<int>> == "ns::Type<int>");
static_assert(vir::refl::class_name<ns::Type<int>> == "ns::Type");

static_assert(vir::refl::reflectable<ns::Type2>);
static_assert(std::same_as<vir::refl::base_type<ns::Type2>, ns::Type<ns::Type2>>);
static_assert(vir::refl::data_member_count<ns::Type2> == 1);
static_assert(vir::refl::class_name<ns::Type2> == "ns::Type2");

static_assert(vir::refl::reflectable<Type3>);
static_assert(vir::refl::class_name<Type3> == "Type3");
static_assert(std::same_as<vir::refl::base_type<Type3>, ns::Type<Type3>>);
static_assert(vir::refl::data_member_count<Type3> == 4);

const char*
string_test()
{ return vir::refl::class_name<ns::Type<int>>.c_str(); }

std::string_view
string_test2()
{ return vir::refl::class_name<ns::Type<float>>; }

const char*
member_name_string0()
{ return vir::refl::data_member_name<ns::Type<char>, 0>.c_str(); }
