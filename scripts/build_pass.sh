#!/bin/bash

#nprocs -1 as building rtlib will use one CPU as well
: ${CPUS:=1}

RTLIBFLAGS="-O2 -g0 -fopenmp -Wunknown-pragmas -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable"
RTLIB_PATH="${CATO_ROOT}/src/cato/rtlib/rtlib.cpp"
RTLIB_OUT="${CATO_ROOT}/src/build/rtlib.bc"

SRC_PATH="${CATO_ROOT}/src"

if [ -n "$1" ] && [ $1 = "--rebuild" ]; then
    rm -rf ${SRC_PATH}/build 
elif [ -n "$1" ]; then
    echo "use --rebuild to first clean build dir"
fi

mkdir -p ${SRC_PATH}/build
cd ${SRC_PATH}/build

# (mpicxx -cxx=clang++ $RTLIBFLAGS -emit-llvm -c -o $RTLIB_OUT $RTLIB_PATH)& pid=$!
echo "Build collection of external C++ functions"
mpicxx -cxx=clang++ $RTLIBFLAGS -emit-llvm -c -o $RTLIB_OUT $RTLIB_PATH

#build the pass
echo "Build LLVM pass"
# cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug ..
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
make -j$CPU
# wait $pid
