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


spack unload --all
spack load --first netcdf-c +mpi ~parallel-netcdf
#spack unload openmpi # macht z.Zt nur Ärger, weil andere Flags und neues Interface
spack load --first mpich@4.0.2
spack load llvm@13.0.0 build_type=RelWithDebInfo_V-MB
#spack load llvm@13.0.0 build_type=Debug_V-MB
spack load --first cmake
spack load gcc@12.1.0 # something goes wront with _ZSt28__throw_bad_array_new_lengthv otherwise

# export LIBS="$(nc-config --libs) ${LIBS}"
#export CATO_LIBS="-lnetcdf"
#export CPLUS_INCLUDE_PATH="$(spack location -i mpich)/include"
# export RPATH_LIBS=",-rpath,$(nc-config --prefix)/lib" ${RPATH_LIBS}
