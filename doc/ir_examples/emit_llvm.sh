#!/bin/bash
CXXFLAGS="-fopenmp -O2"

clang++ $CXXFLAGS -emit-llvm -S -o - $1
