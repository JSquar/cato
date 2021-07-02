#!/bin/bash
CXXFLAGS="-fopenmp -O2"

if [ -z $1 ]; then
clang++ $CXXFLAGS -emit-llvm -S -o - cato/rtlib/rtlib.cpp 
else
clang++ $CXXFLAGS -emit-llvm -S -o - cato/rtlib/rtlib.cpp | grep $1
fi
