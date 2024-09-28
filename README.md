# vir-reflect-light

[![CI](https://github.com/mattkretz/vir-reflect-light/actions/workflows/CI.yml/badge.svg)](https://github.com/mattkretz/vir-reflect-light/actions/workflows/CI.yml)
[![DOI](https://zenodo.org/badge/853507449.svg)](https://zenodo.org/doi/10.5281/zenodo.13762631)
[![OpenSSF Best Practices](https://www.bestpractices.dev/projects/9447/badge)](https://www.bestpractices.dev/projects/9447)
[![REUSE status](https://github.com/mattkretz/vir-reflect-light/actions/workflows/reuse.yml/badge.svg)](https://github.com/mattkretz/vir-reflect-light/actions/workflows/reuse.yml)
[![fair-software.eu](https://img.shields.io/badge/fair--software.eu-%E2%97%8F%20%20%E2%97%8F%20%20%E2%97%8B%20%20%E2%97%8F%20%20%E2%97%8F-yellow)](https://fair-software.eu)

This package tries to be as light on compile-time as possible while enabling 
the following features:

1. List the number of data members.

2. Provide the variable names (identifiers) of the data members as 
   `fixed_string` (which can be used as `std::string_view` via the `.view()` 
   member on `fixed_string`.

3. Access lvalue references to the data members of a given object using either 
   an index or the variable name.

4. Supports any inheritance hierarchy.

5. Allow exposing only a subset of the data members.

## Installation

```sh
make install prefix=/usr
```

## Usage

### The macro `VIR_MAKE_REFLECTABLE`

Add this macro in the public section of a class definition to make all listed 
class data members reflectable. The members that are not listed will be ignored 
and not be reflectable.

`VIR_MAKE_REFLECTABLE(<class name>, [<data member name>, [<data member name>, 
[...]]])`

Example:

```c++
struct Base
{
  int id;
  VIR_MAKE_REFLECTABLE(Base, id);
};

struct Point : Base
{
  float x, y, z;
  VIR_MAKE_REFLECTABLE(Point, x, y, z);
};

namespace yours
{
  template <typename T>
  class Person
  {
    std::string first_name, last_name;
    T height;
    T weight;

  public:
    VIR_MAKE_REFLECTABLE(Person, height, weight);
  };
}
```

In the above `yours::Person<T>` example, note that:

1. VIR_MAKE_REFLECTABLE needs to be public.

2. The class name `Person` in the macro can also be spelled as 
   `yours::Person<T>`, or `Person<T>` but *not* as `yours::Person`.

3. `first_name` and `last_name` will not be seen by the `vir::refl::` API. It 
   is no error to omit them here if you want to hide them from the reflection 
   API.

### `vir::refl::reflectable<T>`

Concept that is satisfied if the class `T` definition contains a valid 
(`public`) `VIR_MAKE_REFLECTABLE` expansion.

### `vir::refl::nttp_name<X>`

A `vir::fixed_string` object identifying the value/name of `X`. For integers 
this will be a string representation of the value. For enums it is equivalent 
to the following facility.

### `vir::refl::enum_name<X>`

A `vir::fixed_string` object identifying the name of `X`. This name includes 
namespaces. For the name of the enum type use the following facility:

### `vir::refl::type_name<T>`

A `vir::fixed_string` object identifying the name of `T`. This name includes 
namespaces and template arguments.

### `vir::refl::class_name<T>`

A `vir::fixed_string` object identifying the class name of `T`. This name 
includes namespaces but no template arguments.

### `vir::refl::base_type<T>`

Alias for the reflectable base type of the type `T`. If no base type exists (is 
known) an alias for `void`.

There is no support for inheriting from multiple reflectable base classes. But 
multiple inheritance is otherwise not a problem. The derived class can also 
make members of the non-reflectable base type reflectable.

Given the above definition of `Point` and `Base`:

- `vir::refl::base_type<Point>` is an alias for `Base<Point>`.

- `vir::refl::base_type<Base>` is an alias for `void`.

### `vir::refl::data_member_count<T>`

A `std::size_t` value identifying the number of reflectable data members.

- `vir::refl::data_member_count<Point>` is `4`.

- `vir::refl::data_member_count<Base>` is `1`.

### `vir::refl::data_member_name<T, Idx>`

- `vir::refl::data_member_name<Base, 0>` is `vir::fixed_string("id")`.

- `vir::refl::data_member_name<Point, 0>` is `vir::fixed_string("id")`.

- `vir::refl::data_member_name<Point, 1>` is `vir::fixed_string("x")`.

- ...

### `vir::refl::data_member_index<T, Name>`

The index value of the data member with the name `Name` of class `T`.
The name must match the name returned by `data_member_name`. It can be given as 
a string literal or `vir::fixed_string_arg`.

### `vir::refl::data_member_type<T, Idx>` / `data_member_type<T, Name>`

Alias for the type of the data member with index `Idx` or name `Name` of class 
`T`. `data_member_index<T, Name>` is used for mapping a name to an index.

### `vir::refl::data_member<Idx>(obj)` / `data_member<Name>(obj)`

Returns a (const) lvalue-reference to the member of `obj` at the given index 
`Idx` / with the given name `Name`. `data_member_index<T, Name>` is used for 
mapping a name to an index.

### `vir::refl::all_data_members(obj)`

Returns a `vir::simple_tuple<...>` of references to all the reflectable data 
members of `obj`.

### `vir::refl::find_data_members<T, Predicate>`

A `constexpr std::array<size_t, N>` identifying all data member indices that 
match `Predicate`. `Predicate` needs to be a template with two template 
arguments:

1. A `typename` that simply passes the `T` argument from `find_data_members` 
   on.
2. A `size_t` index identifying the data member to consider.

Example:

```c++
struct A {
  int foo;
  double x, y, z;
  float angle;
};

template <typename T, size_t Idx>
using sizeof4
  = std::bool_constant<sizeof(vir::refl::data_member_type<T, Idx>) == 4>;

constexpr std::array idxs = vir::refl::find_data_members<A, sizeof4>;
// => idxs == {0, 4}

template <typename T, size_t Idx>
using int_or_angle
  = std::disjunction<std::bool_constant<vir::refl::data_member_name<T, Idx> == "angle">,
                     std::same_as<vir::refl::data_member_type<T, Idx>, int>>;

constexpr std::array idx2 = vir::refl::find_data_members<A, int_or_angle>;
// => idx2 == {0, 4}
```

### `vir::refl::find_data_members_by_type<T, Predicate>`

Shorthand for the above so that standard type traits can be used as 
`Predicate`. `Predicate<data_member_type<T, Idx>>` determines whether the index 
`Idx` is returned from the array. The above `sizeof4` predicate can thus be 
written as:

```c++
template <typename T>
using sizeof4 = std::bool_constant<sizeof(T) == 4>;

constexpr std::array idxs = vir::refl::find_data_members_by_type<A, sizeof4>;
// => idxs == {0, 4}
```

### `vir::refl::data_member_types<T, IndexArray = /*all*/>`

Alias for a `vir::simple_tuple<...>` type where the tuple types are equal to 
the data member types of `T` at the indexes given by `IndexArray`. If 
`IndexArray` is omitted, all data members will be listed.

### `vir::refl::for_each_data_member_index<T>(callable)`

Calls the function `callable` with one argument of type 
`std::integral_constant<std::size_t, i>`. The function is called for each `i` 
in the half-open range `0` to `data_member_count<T>`.
