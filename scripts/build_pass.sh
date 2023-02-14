#!/usr/bin/env bash
###
# Author: Michael Blesel
# -----
#Last Modified: Wednesday, 8th February 2023 10:15:35 am
#Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
# -----
# Copyright (c) 2020 Michael Blesel
# 
###
#!/bin/bash

#nprocs -1 as building rtlib will use one CPU as well
: ${CPUS:=1}
#: ${CATO_DEBUG:=0}

YELLOW='\033[1;33m'
NC='\033[0m' # No Color

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

echo "+++++++++++"
echo -e "${YELLOW}Used MPI:${NC} $(which mpicc)"
echo -e "${YELLOW}Used clang++:${NC} $(which clang++)"
echo "+++++++++++"

# (mpicxx -cxx=clang++ $RTLIBFLAGS -emit-llvm -c -o $RTLIB_OUT $RTLIB_PATH)& pid=$!
echo "Build collection of external C++ functions"
mpicxx -cxx=clang++ $RTLIBFLAGS -emit-llvm -c -o $RTLIB_OUT $RTLIB_PATH

#build the pass
echo "+++++++++++"
echo -e "${YELLOW}Build LLVM pass${NC}"
echo "+++++++++++"

if [[ ${CATO_DEBUG} ]]; then
    echo -e "${YELLOW}WITH debug statements${NC}"
    sed -i "s/\#define DEBUG_CATO_PASS 0/\#define DEBUG_CATO_PASS 1/" ${SRC_PATH}/cato/debug.h
else
    echo -e "${YELLOW}WITHOUT debug statements${NC}"
    sed -i "s/\#define DEBUG_CATO_PASS 1/\#define DEBUG_CATO_PASS 0/" ${SRC_PATH}/cato/debug.h
fi
# cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug ..
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
make -j$CPU
# wait $pid
