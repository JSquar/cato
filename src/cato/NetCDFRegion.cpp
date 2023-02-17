/*
 * File: NetCDFRegion.cpp
 * Created: Thursday, 16th February 2023 11:27:28 am
 * Author: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * 
 * -----
 * Last Modified: Thursday, 16th February 2023 12:28:52 pm
 * Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Jannek Squar
 * 
 */
#include "NetCDFRegion.h"

NetCDFRegion::NetCDFRegion(llvm::CallInst *open_call)
{
    llvm::Function *open_func = open_call->getCalledFunction();
    assert(open_func != nullptr && "ERROR: Allocation function could not be found");
    llvm::StringRef open_func_name = open_func->getName();

    Debug(llvm::errs() << "NetCDF sequential open call: ";); 
    Debug(open_call->dump(););    
    Debug(llvm::errs() << "Used open function: " << open_func_name << "\n";);

    _path = open_call->getArgOperand(0);
    Debug(llvm::errs() << "Path of file:\n";);
    Debug(_path->dump(););

    _omode = open_call->getArgOperand(1);
    Debug(llvm::errs() << "Modification mode:\n";);
    Debug(_omode->dump(););
    
    _ncid = open_call->getArgOperand(2);
    Debug(llvm::errs() << "ncid:\n";);
    Debug(_ncid->dump(););
    
}

llvm::Value *NetCDFRegion::NetCDFRegion::get_ncid() {
  return _ncid;
}

llvm::Value *NetCDFRegion::NetCDFRegion::get_omode() {
  return _omode;
}

llvm::Value *NetCDFRegion::NetCDFRegion::get_path() {
  return _path;
}

llvm::CallInst *NetCDFRegion::get_call() {
  return _open_call;
}

// MemoryAllocation::MemoryAllocation(CallInst *call_inst)
// {


    

//     if (alloc_func_name.equals("malloc"))
//     {
       
//     }

//     // Try to find the type of the returned pointer
//     // In normal cases the instruction directly after the memory allocation
//     // should be a bitcast to its destined type
//     if (auto bitcast_inst = dyn_cast<BitCastInst>(call_inst->getNextNode()))
//     {
//         if (bitcast_inst->getOperand(0) == call_inst)
//         {
//             _allocation_type = bitcast_inst->getDestTy();
//         }
//     }
//     else
//     {
//         _allocation_type = call_inst->getType();
//     }
//     Debug(_allocation_type->dump(););
// }