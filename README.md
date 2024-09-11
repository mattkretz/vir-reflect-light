# vir-reflect-light

[![CI](https://github.com/mattkretz/vir-reflect-light/actions/workflows/CI.yml/badge.svg)](https://github.com/mattkretz/vir-reflect-light/actions/workflows/CI.yml)
[![fair-software.eu](https://img.shields.io/badge/fair--software.eu-%E2%97%8F%20%20%E2%97%8F%20%20%E2%97%8B%20%20%E2%97%8B%20%20%E2%97%8B-orange)](https://fair-software.eu)

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

Add this macro inside of a class definition to make all listed class 
(non-static) data members reflectable.

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
```

### `vir::refl::reflectable<T>`

Concept that is satisfied if the class `T` definition contains a valid 
`VIR_MAKE_REFLECTABLE` expansion.

### `vir::refl::type_name<T>`

A `vir::fixed_string` object identifying the name of `T`. This name includes 
namespaces and template arguments.

### `vir::refl::class_name<T>`

A `vir::fixed_string` object identifying the class name of `T`. This name 
includes namespaces but no template arguments.

### `vir::refl::base_type<T>`

Alias for the base type of the type `T`. Does not support multiple inheritance. 
If no base type exists (is known) an alias for `void`.

- `vir::refl::base_type<Point>` is an alias for `Base<Point>`.

- `vir::refl::base_type<Base>` is an alias for `void`.

### `vir::refl::data_member_count<T>`

- `vir::refl::data_member_count<Point>` is `4`.

- `vir::refl::data_member_count<Base>` is `1`.

### `vir::refl::data_member_name<T, Idx>`

- `vir::refl::data_member_name<Base, 0>` is `vir::fixed_string("id")`.

- `vir::refl::data_member_name<Point, 0>` is `vir::fixed_string("id")`.

- `vir::refl::data_member_name<Point, 1>` is `vir::fixed_string("x")`.

- ...

### `vir::refl::data_member_type<T, Idx>`, `data_member_type<T, Name>`

Alias for the type of the data member with index `Idx` or name `Name` of class 
`T`.

### `vir::refl::data_member<Idx>(obj)`

Returns a (const) lvalue-reference to the member of `obj` at the given index.

### `vir::refl::data_member<Name>(obj)`

Returns a (const) lvalue-reference to the member of `obj` with the given name. 
The name must match the name returned by `data_member_name`. It can be given as 
string literal or `vir::fixed_string`.
