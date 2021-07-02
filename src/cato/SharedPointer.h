#ifndef CATO_SHARED_POINTER_H
#define CATO_SHARED_POINTER_H

#include <llvm/IR/Function.h>
#include <llvm/IR/Value.h>

#include "SharedVariable.h"

class SharedPointer : public SharedVariable
{
  private:
    int _pointer_depth;

  public:
    SharedPointer(llvm::Value *value, llvm::Function *microtask, int);

    ~SharedPointer(){};
};
#endif
