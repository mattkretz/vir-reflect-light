/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2024      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                       Matthias Kretz <m.kretz@gsi.de>
 */
#include <vir/reflect-light.h>

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
