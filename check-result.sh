#!/bin/sh
symbols="$(nm -SC test.o)"
symbol_count=$(echo "$symbols"|wc -l)
string_count=$(echo "$symbols"|grep -c ' [Vu] .*string_storage')
rodata_size_arr=$(echo "$symbols"|grep '[0-9a-zA-Z] [Vur] \<'|cut -d' ' -f2|sed 's/^0*//')
rodata_size=0
for n in $rodata_size_arr; do
  rodata_size=$((rodata_size + n))
done
function_count=$(echo "$symbols"|grep -c ' T ')
rodata=$(objdump -s test.o|grep --no-group-separator -A1 'section .rodata'|grep -v 'section .rodata'|sed 's/^.*  \<\(.*\)\. *$/\1/'|sort|sed 's/$/,/')
rodata="$(echo $rodata)"
rodata="${rodata%,}"

failed=false

print_line() {
  printf "%14s | %${rodata_size}s | %${rodata_size}s %s\n" "$1" "$3" "$2" "$4"
}

one_test() {
  mark="✔️"
  if test "$2" != "$3"; then
    mark="❌"
    failed=true
  fi
  print_line "$@" "$mark"
}

print_line Test Result Expected
#echo "---------------+----------+----------"
one_test "# symbols" $symbol_count 7
one_test "# functions" $function_count 5
one_test "# opt. strings" $string_count 2
one_test "# other const" $(echo "$symbols"|grep -c ' r ') 0
one_test "sizeof .rodata" $rodata_size $((9+5))
one_test ".rodata" "$rodata" "blah, ns::Type"
echo

if $failed; then
  echo "=> FAILED."
  exit 1
fi

echo "=> PASSED."
