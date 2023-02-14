#!/usr/bin/env bash

# Pass Debugging
# gdb --args opt ....
# 

#  Manuelle Ausführung:
#  clang -S -emit-llvm -fopenmp basic_omp.c
#  opt -load-pass-plugin=$HOME/Git/04_LLVM-Translation-Tool/cato/src/build/cato/libCatoPass.so -passes="Cato" basic_omp.ll -S > basic_omp_modified.ll llvm-as basic_omp_modified.ll -o
#  basic_omp_modified.bc mpicc -cc=clang -o basic_omp_modified.x basic_omp_modified.bc $HOME/Git/04_LLVM-Translation-Tool/cato/src/build/cato/rtlib/libCatoRuntime.so

set -e

PASS_PATH="${CATO_ROOT}/src/build/cato/libCatoPass.so"
RTLIB_PATH="${CATO_ROOT}/src/build/cato/rtlib/libCatoRuntime.so"

clang -S -emit-llvm -fopenmp ${1}.c
opt -load-pass-plugin=${PASS_PATH} -passes="Cato" ${1}.ll -S > ${1}_modified.ll
llvm-as ${1}_modified.ll -o ${1}_modified.bc
mpicc -cc=clang -o ${1}_modified.x ${1}_modified.bc ${RTLIB_PATH}