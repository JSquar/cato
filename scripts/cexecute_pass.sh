#!/bin/bash
[ "$DEBUG" == 'true' ] && set -x

if [ -z $1 ]; then
    echo "Error: no input file given"
    exit 1
fi

OUT=translated
LOGGING=""


if [ -n "$2" ] && [ $2 = "--logging" ]; then
    echo "Logging has been enabled"
    LOGGING="-mllvm --cato-logging"
elif [ -n "$2" ]; then
    echo "Output name $2"
    OUT=$2
fi

CXXFLAGS="-O2 -g0 -fopenmp -Wunknown-pragmas -Wall -Wextra"

PASS_PATH="${CATO_ROOT}/src/build/cato/libCatoPass.so"
RTLIB_DIR="${CATO_ROOT}/src/build/cato/rtlib"

# mpicc -cc=clang $CXXFLAGS -o $OUT.o -Xclang -load -Xclang $PASS_PATH $LOGGING -c $1  
# mpicc -cc=clang $CXXFLAGS -o $OUT $OUT.o ${RTLIB_DIR}/libCatoRuntime.so -Wl,-rpath,${RTLIB_DIR}
# source: https://stackoverflow.com/questions/54447985/how-to-automatically-register-and-load-modern-pass-in-clang
mpicc -cc=clang $CXXFLAGS -o $OUT.o -fpass-plugin=$PASS_PATH $LOGGING -c $1  
mpicc -cc=clang $CXXFLAGS -o ${OUT}_translated.ll -fpass-plugin=$PASS_PATH $LOGGING $1 -S -emit-llvm
mpicc -cc=clang $CXXFLAGS -o $OUT $OUT.o ${RTLIB_DIR}/libCatoRuntime.so -Wl,-rpath,${RTLIB_DIR}

rm $OUT.o
