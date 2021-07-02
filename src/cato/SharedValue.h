#ifndef CATO_SHARED_VALUE_H
#define CATO_SHARED_VALUE_H

#include <llvm/IR/Function.h>
#include <llvm/IR/Value.h>

#include "SharedVariable.h"

class SharedValue : public SharedVariable
{
  private:
  public:
    SharedValue(llvm::Value *value, llvm::Function *microtask);

    ~SharedValue(){};
};

#endif
