#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2024      GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
#                       Matthias Kretz <m.kretz@gsi.de>
symbols="$(nm -SC test.o|grep -v ' U __gxx_personality_v0')"
symbol_count=$(echo "$symbols"|wc -l)
string_count=$(echo "$symbols"|grep -c ' [Vu] template parameter object for vir::fixed_string<')
rodata_size_arr=$(echo "$symbols"|grep '[0-9a-zA-Z] [Vur] \<'|cut -d' ' -f2|sed 's/^0*//')
rodata_size=0
for n in $rodata_size_arr; do
  rodata_size=$((rodata_size + n))
done
function_count=$(echo "$symbols"|grep -c ' T ')

rodata=$(
nm -S test.o|grep '[0-9a-zA-Z] [Vur] \<'|cut -d' ' -f4|while read sym; do
printf "%s, \n" $(objdump --no-addresses --no-show-raw-insn -dj .rodata.$sym test.o | grep '^	' | cut -f2-)
done|sort
)

rodata="${rodata%, }"
rodata=$(echo $rodata)

failed=false

field_size=$((rodata_size+2*(string_count-1)))
print_line() {
  printf "%15s | %${field_size}s | %${field_size}s %s\n" "$1" "$3" "$2" "$4"
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
one_test "# fixed_strings" $string_count 2
one_test "# other const" $(($(echo "$symbols"|grep -c ' [rVu] ')-string_count)) 0
one_test "sizeof .rodata" $rodata_size $((8 + 4))
one_test ".rodata" "$rodata" "blah, ns::Type"
printf "%15s : %d Bytes\n " ".o filesize" $(stat -c %s test.o)

if $failed; then
  echo "=> FAILED."
  exit 1
fi

echo "=> PASSED."
