#!/bin/bash
CPUS=$(nproc)
#nprocs -1 as building rtlib will use one CPU as well
let CPUS-=1

RTLIBFLAGS="-O2 -g0 -fopenmp -Wunknown-pragmas -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable"
RTLIB_PATH="${DAS_TOOL_ROOT}/src/cato/rtlib/rtlib.cpp"
RTLIB_OUT="${DAS_TOOL_ROOT}/src/build/rtlib.bc"

SRC_PATH="${DAS_TOOL_ROOT}/src"

if [ -n "$1" ] && [ $1 = "--rebuild" ]; then
    rm -rf ${SRC_PATH}/build 
elif [ -n "$1" ]; then
    echo "use --rebuild to first clean build dir"
fi

mkdir -p ${SRC_PATH}/build
cd ${SRC_PATH}/build

(mpicxx -cxx=clang++ $RTLIBFLAGS -emit-llvm -c -o $RTLIB_OUT $RTLIB_PATH)& pid=$!

#build the pass
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
make -j$CPU
wait $pid
