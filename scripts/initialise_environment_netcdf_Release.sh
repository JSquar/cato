#!/usr/bin/env bash
###
# File: initialise_environment.sh
# Created: Monday, 30th August 2021 5:06:45 pm
# Author: Jannek Squar (jannek.squar@uni-hamburg.de)
# -----
# 
# -----
#Last Modified: Friday, 17th February 2023 11:52:03 am
#Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
# -----
# Copyright (c) 2021 UHH
# 
###
#set -e

spack unload --all
spack load --first netcdf-c@4.9:+mpi~parallel-netcdf^mpich@4:
#spack load --first mpich@4:
spack load llvm@13.0.0 build_type=Release/tpgvmvv || spack load llvm@13.0.0 build_type=Release
spack load --first cmake
spack load googletest
spack load gcc@12.1.0 || echo "Could not load gcc@12.1.0" # something goes wront with _ZSt28__throw_bad_array_new_lengthv otherwise

