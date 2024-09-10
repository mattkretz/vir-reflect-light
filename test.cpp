/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2024      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */

#include <vir/reflect-light.h>
#include <vir/simple_tuple.h>
#include <utility>

namespace d = vir::refl::detail;

[[maybe_unused]] constexpr auto i0 = d::ic<0>;
[[maybe_unused]] constexpr auto i1 = d::ic<1>;
[[maybe_unused]] constexpr auto i2 = d::ic<2>;
[[maybe_unused]] constexpr auto i3 = d::ic<3>;
/*[[maybe_unused]] constexpr auto i4 = d::ic<4>;
[[maybe_unused]] constexpr auto i5 = d::ic<5>;
[[maybe_unused]] constexpr auto i6 = d::ic<6>;
[[maybe_unused]] constexpr auto i7 = d::ic<7>;
[[maybe_unused]] constexpr auto i8 = d::ic<8>;
[[maybe_unused]] constexpr auto i9 = d::ic<9>;
[[maybe_unused]] constexpr auto i10 = d::ic<10>;
[[maybe_unused]] constexpr auto i11 = d::ic<11>;
[[maybe_unused]] constexpr auto i12 = d::ic<12>;
[[maybe_unused]] constexpr auto i13 = d::ic<13>;
[[maybe_unused]] constexpr auto i14 = d::ic<14>;
[[maybe_unused]] constexpr auto i15 = d::ic<15>;*/

static_assert(vir::simple_tuple {1, 2} != vir::simple_tuple {2, 3});
static_assert(vir::simple_tuple {1, 2}[std::integral_constant<int, 0>()] == 1);
static_assert(std::same_as<decltype(vir::simple_tuple {1, 2}[i0]), int&&>);
static_assert(vir::simple_tuple {} == vir::simple_tuple {});
static_assert(vir::simple_tuple {} <= vir::simple_tuple {});

static_assert([] {
  vir::simple_tuple t3{1, 2, 3};
  static_assert(std::same_as<decltype(t3[i0]), int&>);
  static_assert(std::same_as<decltype(std::as_const(t3)[i0]), int const&>);
  int a = 0, b = 1, c = 2, d = 3;
  auto t4 = vir::tie(a, b, c, d);
  if (not(t4[i0] == 0 and t4[i1] == 1) and t4[i2] == 2 and t4[i3] == 3)
    return false;
  a = 4;
  if (t4[i0] != 4)
    return false;
  if (&t4[i0] != &a)
    return false;
  return true;
}());

static_assert(vir::refl::type_name<int> == vir::fixed_string<"int">());
static_assert(vir::refl::type_name<float> == "float");

namespace ns0
{
  template <typename T, auto X>
    struct NotReflected
    {};
}

static_assert(vir::refl::class_name<ns0::NotReflected<int, 5.f>> == "ns0::NotReflected");
#ifdef __clang__
static_assert(vir::refl::type_name<ns0::NotReflected<int, 5.f>> == "ns0::NotReflected<int, 5.000000e+00>");
#else
static_assert(vir::refl::type_name<ns0::NotReflected<int, 5.f>> == "ns0::NotReflected<int, 5.0e+0f>");
#endif

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

static_assert(vir::refl::is_reflectable<Derived>);
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
  return true;
}());

struct Further : Derived
{
  char c;
  VIR_MAKE_REFLECTABLE(Further, c);
};

static_assert(vir::refl::is_reflectable<Further>);
static_assert(std::same_as<vir::refl::base_type<Further>, Derived>);

struct AndAnother : Further
{
  VIR_MAKE_REFLECTABLE(AndAnother);
};

static_assert(vir::refl::is_reflectable<AndAnother>);
static_assert(std::same_as<vir::refl::base_type<AndAnother>, Further>);

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

static_assert(vir::refl::is_reflectable<ns::Type<int>>);
static_assert(std::same_as<vir::refl::base_type<ns::Type<int>>, void>);
static_assert(vir::refl::data_member_count<ns::Type<int>> == 1);
static_assert(vir::refl::type_name<ns::Type<int>> == "ns::Type<int>");
static_assert(vir::refl::class_name<ns::Type<int>> == "ns::Type");

static_assert(vir::refl::is_reflectable<ns::Type2>);
static_assert(std::same_as<vir::refl::base_type<ns::Type2>, ns::Type<ns::Type2>>);
static_assert(vir::refl::data_member_count<ns::Type2> == 1);
static_assert(vir::refl::class_name<ns::Type2> == "ns::Type2");

const char*
string_test()
{ return vir::refl::class_name<ns::Type<int>>.c_str(); }

std::string_view
string_test2()
{ return vir::refl::class_name<ns::Type<float>>; }

const char*
member_name_string0()
{ return vir::refl::data_member_name<ns::Type<char>, 0>.c_str(); }
