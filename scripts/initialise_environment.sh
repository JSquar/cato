#!/usr/bin/env bash
###
# File: initialise_environment.sh
# Created: Monday, 30th August 2021 5:06:45 pm
# Author: Jannek Squar (jannek.squar@uni-hamburg.de)
# -----
# 
# -----
# Last Modified: Wednesday, 29th June 2022 1:19:56 pm
# Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
# -----
# Copyright (c) 2021 UHH
# 
###


spack unload --all
spack load --first netcdf-c +mpi ~parallel-netcdf
#spack unload openmpi # macht z.Zt nur Ärger, weil andere Flags und neues Interface
spack load --first mpich@4.0.2
spack load llvm@13.0.0 build_type=RelWithDebInfo_V-MB
spack load --first cmake

# export LIBS="$(nc-config --libs) ${LIBS}"
#export CATO_LIBS="-lnetcdf"
#export CPLUS_INCLUDE_PATH="$(spack location -i mpich)/include"
# export RPATH_LIBS=",-rpath,$(nc-config --prefix)/lib" ${RPATH_LIBS}
