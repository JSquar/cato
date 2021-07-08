#!/bin/bash
[ "$DEBUG" == 'true' ] && set -x
set -u

i=0
# formats all code with clang-format

(clang-format -i -style=file $CATO_ROOT/src/cato/*.cpp ; clang-format -i -style=file $CATO_ROOT/src/cato/*.h )&
pids[${i}]=$!
    let i++

(clang-format -i -style=file $CATO_ROOT/src/cato/rtlib/*.cpp ; clang-format -i -style=file $CATO_ROOT/src/cato/rtlib/*.h )&
pids[${i}]=$!
    let i++

for pid in ${pids[*]}; do
    wait $pid
done
