/*
 * File: netCDFHandler.cpp
 * Created: Thursday, 16th February 2023 12:08:20 pm
 * Author: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * 
 * -----
 * Last Modified: Thursday, 16th February 2023 6:07:14 pm
 * Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Jannek Squar
 * 
 */
#include "netCDFHandler.h"

void replace_sequential_netCDF(llvm::Module &M){
    std::vector<llvm::CallInst *> open_instructions = find_netcdf(M);
    std::vector<std::unique_ptr<NetCDFRegion>> netcdf_regions = get_netcdf_region(open_instructions);
}

std::vector<std::unique_ptr<NetCDFRegion>> get_netcdf_region(std::vector<llvm::CallInst *> &call_instructions) {
    std::vector<std::unique_ptr<NetCDFRegion>> netcdf_regions;

    for (auto &instruction : call_instructions)
    {
        std::unique_ptr<NetCDFRegion> netcdf_region = std::make_unique<NetCDFRegion>(NetCDFRegion(instruction));
        netcdf_regions.push_back(std::move(netcdf_region));
    }
    
    return netcdf_regions;    
}

std::vector<llvm::CallInst *> find_netcdf(llvm::Module &M)
{
    llvm::errs() << "Function: find_netcdf\n";
    std::vector<llvm::CallInst *> open_instructions;

    std::vector<llvm::User *>  open_users = get_function_users(M, "nc_open");
    Debug(llvm::errs() << "Found " << open_users.size() << " many open calls\n";);

    for (auto &user : open_users)
    {
        if (auto *call = llvm::dyn_cast<llvm::CallInst>(user))
        {
            open_instructions.push_back(call);
        }
    }

    return open_instructions;
}