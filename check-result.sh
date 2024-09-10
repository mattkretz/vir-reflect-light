#!/bin/sh
symbols="$(nm -SC test.o)"
symbol_count=$(echo "$symbols"|wc -l)
string_count=$(echo "$symbols"|grep -c string_value)
string_size=$((1 * $(echo "$symbols"|grep string_value|cut -d' ' -f2|sed 's/^0*//')))
function_count=$(echo "$symbols"|grep -c ' T ')
rodata=$(objdump -s test.o|grep -A1 'section .rodata'|tail -n1|sed 's/^.*  \<\(.*\)\. *$/\1/')

failed=false

print_line() {
  printf "%14s | %8s | %8s %s\n" "$1" "$3" "$2" "$4"
}

one_test() {
  mark="✔️"
  if test $2 != $3; then
    mark="❌"
    failed=true
  fi
  print_line "$@" "$mark"
}

print_line Test Result Expected
echo "---------------+----------+----------"
one_test "# symbols" $symbol_count 5
one_test "# functions" $function_count 4
one_test "# strings" $string_count 1
one_test "sizeof string" $string_size 9
one_test ".rodata" $rodata "ns::Type"
echo

if $failed; then
  echo "=> FAILED."
  exit 1
fi

echo "=> PASSED."
