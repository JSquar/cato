#!/usr/bin/env bash
###
# File: initialise_environment.sh
# Created: Monday, 30th August 2021 5:06:45 pm
# Author: Jannek Squar (jannek.squar@uni-hamburg.de)
# -----
# 
# -----
#Last Modified: Wednesday, 8th February 2023 11:06:22 am
#Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
# -----
# Copyright (c) 2021 UHH
# 
###

#: {DEBUG:=0}

spack unload --all
spack load --first netcdf-c
spack unload openmpi # macht z.Zt nur Ärger, weil andere Flags und neues Interface
spack load mpich@3.3.1
# spack load mpich@4.0.2

YELLOW='\033[1;33m'
NC='\033[0m' # No Color

if [[ ${CATO_DEBUG} ]]; then
  echo -e "${YELLOW}Load llvm WITH debug scope${NC}"
  spack load llvm@14.0.6 build_type=Debug
# elif [[ ${BACKUP} ]]; then
#   echo -e "${YELLOW}Load llvm WITHOUT debug scope${NC}"
#   spack load llvm@14.0.x build_type=RelWithDebInfo
elif [[ -z ${CATO_DEBUG} ]]; then
  echo -e "${YELLOW}Load llvm without large debug scope${NC}"
  spack load llvm@14.0.6 build_type=RelWithDebInfo
else
  echo "Should not have said that"
  return
fi

spack load --first cmake

# export LIBS="$(nc-config --libs) ${LIBS}"
export CATO_LIBS="-lnetcdf"
# export CPLUS_INCLUDE_PATH="$(spack location -i mpich)/include"
# export RPATH_LIBS=",-rpath,$(nc-config --prefix)/lib" ${RPATH_LIBS}
