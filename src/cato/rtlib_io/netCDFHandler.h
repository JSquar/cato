/*
 * File: IOAbstractionHandler.h
 * Created: Wednesday, 15th February 2023 6:25:12 pm
 * Author: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 *
 * -----
 * Last Modified: Thursday, 16th February 2023 6:02:01 pm
 * Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Jannek Squar
 *
 */
#ifndef netCDFHandler
#define netCDFHandler

#include <llvm/IR/Module.h>
#include "../RuntimeHandler.h"
#include <llvm/ADT/StringRef.h>
#include "../helper.h"
#include "netCDFHandler.h"
#include "../debug.h"
#include "NetCDFRegion.h"

void replace_sequential_netCDF(llvm::Module &);

std::vector<std::unique_ptr<NetCDFRegion>> get_netcdf_region(std::vector<llvm::CallInst*> &call_instructions);    

std::vector<llvm::CallInst *> find_netcdf(llvm::Module &M);

#endif /* netCDFHandler */
