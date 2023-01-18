#!/bin/bash
[ "$DEBUG" == 'true' ] && set -x

if [ -z $1 ]; then
    echo "Error: no input file given"
    exit 1
fi

OUT=$2

if [ -z $2 ]; then
    echo "No output name specified. Compiled binary will be named \'translated\'"
    OUT=translated
fi

CXXFLAGS="-O2 -g0 -fopenmp -Wunknown-pragmas"

PASS_PATH="${CATO_ROOT}/src/build/cato/libCatoPass.so"
RTLIB_DIR="${CATO_ROOT}/src/build/cato/rtlib"

# mpicxx -cxx=clang++ $CXXFLAGS -o $OUT.o -Xclang -load -Xclang $PASS_PATH -c $1  
mpicxx -cxx=clang++ $CXXFLAGS -o $OUT.o -Xclang -load-pass-plugin -Xclang $PASS_PATH -c $1  
mpicxx -cxx=clang++ $CXXFLAGS -o $OUT $OUT.o ${RTLIB_DIR}/libCatoRuntime.so -Wl,-rpath,${RTLIB_DIR}

rm $OUT.o
