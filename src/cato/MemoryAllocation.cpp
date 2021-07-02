#include "MemoryAllocation.h"

#include <llvm/IR/Function.h>

#include "debug.h"

using namespace llvm;

MemoryAllocation::MemoryAllocation(CallInst *call_inst)
{
    _allocation_call = call_inst;
    Function *alloc_func = call_inst->getCalledFunction();
    assert(alloc_func != nullptr && "ERROR: Allocation function could not be found");
    StringRef alloc_func_name = alloc_func->getName();

    Debug(errs() << "Memory Allocation: ";);
    Debug(call_inst->dump(););
    Debug(errs() << "Used allocation function: " << alloc_func_name << "\n";);

    if (alloc_func_name.equals("malloc"))
    {
        _allocation_size = call_inst->getArgOperand(0);
        Debug(errs() << "Allocation size: ";);
        Debug(_allocation_size->dump(););
    }

    // Try to find the type of the returned pointer
    // In normal cases the instruction directly after the memory allocation
    // should be a bitcast to its destined type
    if (auto bitcast_inst = dyn_cast<BitCastInst>(call_inst->getNextNode()))
    {
        if (bitcast_inst->getOperand(0) == call_inst)
        {
            _allocation_type = bitcast_inst->getDestTy();
        }
    }
    else
    {
        _allocation_type = call_inst->getType();
    }
    Debug(_allocation_type->dump(););
}

CallInst *MemoryAllocation::get_allocation_call() { return _allocation_call; }

Value *MemoryAllocation::get_allocation_size() { return _allocation_size; }

Type *MemoryAllocation::get_allocation_type() { return _allocation_type; }
