#!/usr/bin/env bash
###
# Author: Michael Blesel
# -----
#Last Modified: Thursday, 19th January 2023 2:49:57 pm
#Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
# -----
# Copyright (c) 2020 Michael Blesel
# 
###
#!/bin/bash

#nprocs -1 as building rtlib will use one CPU as well
: ${CPUS:=1}
: ${DEBUG:=0}

RTLIBFLAGS="-O2 -g0 -fopenmp -Wunknown-pragmas -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable"
RTLIB_PATH="${CATO_ROOT}/src/cato/rtlib/rtlib.cpp"
RTLIB_OUT="${CATO_ROOT}/src/build/rtlib.bc"

SRC_PATH="${CATO_ROOT}/src"

if [ -n "$1" ] && [ $1 = "--rebuild" ]; then
    echo "Delete old build dir"
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
if [[ ${DEBUG} ]]; then
    echo "With debug statements"
    sed "s/\#define DEBUG_CATO_PASS 0/\#define DEBUG_CATO_PASS 1/" src/cato/debug.h
else
    echo "Without debug statements"
    sed "s/\#define DEBUG_CATO_PASS 1/\#define DEBUG_CATO_PASS 0/" src/cato/debug.h
fi
# cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug ..
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
make -j$CPU
# wait $pid
