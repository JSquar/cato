#ifndef CATO_MEMORY_ALLOCATION_H
#define CATO_MEMORY_ALLOCATION_H

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

/**
 * Each instance of this class represents a memory allocation function call (e.g. malloc)
 * The type and size (in bytes) of the allocated memory is also stored
 *
 * Currently supported memory allocation functions:
 *  - malloc
 *
 * TODO add more memory allocation functions
 **/
class MemoryAllocation
{
  private:
    llvm::CallInst *_allocation_call;

    llvm::Value *_allocation_size;

    llvm::Type *_allocation_type;

  public:
    /**
     * Constructor takes CallInst* to one of the supported memory allocation functions
     **/
    MemoryAllocation(llvm::CallInst *);

    ~MemoryAllocation(){};

    llvm::CallInst *get_allocation_call();

    llvm::Value *get_allocation_size();

    llvm::Type *get_allocation_type();
};

#endif
