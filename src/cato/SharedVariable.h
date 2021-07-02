#ifndef CATO_SHARED_VARIABLE_H
#define CATO_SHARED_VARIABLE_H

#include <llvm/IR/Function.h>
#include <llvm/IR/Value.h>

class SharedVariable
{
  private:
    llvm::Value *_value;

    llvm::Function *_microtask;

  public:
    SharedVariable(llvm::Value *value, llvm::Function *microtask);

    ~SharedVariable(){};
};

#endif
