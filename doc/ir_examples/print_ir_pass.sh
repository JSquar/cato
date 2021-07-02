#!/bin/bash
clang++ -fopenmp -O2 -Xclang -load -Xclang ./PrintModulePass.so $1
