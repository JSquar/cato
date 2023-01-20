#!/usr/bin/env bash
###
# File: initialise_environment.sh
# Created: Monday, 30th August 2021 5:06:45 pm
# Author: Jannek Squar (jannek.squar@uni-hamburg.de)
# -----
# 
# -----
#Last Modified: Thursday, 19th January 2023 3:19:21 pm
#Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
# -----
# Copyright (c) 2021 UHH
# 
###

: {DEBUG:=0}

spack unload --all
spack load --first netcdf-c
spack unload openmpi # macht z.Zt nur Ärger, weil andere Flags und neues Interface
spack load mpich

if [[ ${DEBUG} ]]; then
  echo "Load llvm with large debug scope"
  spack load llvm@14.0.6 build_type=Debug
elif [[ -z ${DEBUG} ]]; then
  echo "Load llvm without large debug scope"
  spack load llvm@14.0.6 build_type=RelWithDebInfo
else
  echo "Should not have said that"
  return
fi

spack load --first cmake

# export LIBS="$(nc-config --libs) ${LIBS}"
export CATO_LIBS="-lnetcdf"
export CPLUS_INCLUDE_PATH="$(spack location -i mpich)/include"
# export RPATH_LIBS=",-rpath,$(nc-config --prefix)/lib" ${RPATH_LIBS}
