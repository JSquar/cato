/*
 * File: NetCDFRegion.h
 * Created: Thursday, 16th February 2023 11:23:22 am
 * Author: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * 
 * -----
 * Last Modified: Thursday, 16th February 2023 7:34:05 pm
 * Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Jannek Squar
 * 
 */
#ifndef NET_CDFREGION
#define NET_CDFREGION

#include <llvm/IR/Instructions.h>
#include "debug.h"
/**
 * Each instance of this class represents the scope of an interaction with a netCDF file
 **/
class NetCDFRegion
{
  private:
    llvm::CallInst *_open_call;

    llvm::Value *_ncid;

    llvm::Value *_path;

    llvm::Value *_omode;


  public:
    /**
     * Constructor takes CallInst* to one of the supported memory allocation functions
     **/
    NetCDFRegion(llvm::CallInst *);

    ~NetCDFRegion(){};

    llvm::Value *get_ncid();
    llvm::Value *get_path();
    llvm::Value *get_omode();
    
    llvm::CallInst *get_call();

};

#endif /* NET_CDFREGION */
