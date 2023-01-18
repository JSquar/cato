#ifndef CATO_MICROTASK_H
#define CATO_MICROTASK_H

#include <memory>
#include <vector>

#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>

#include "helper.h"

#include "SharedVariable.h"

/**
 * Struct with pointers to OpenMP Runtime Library calls for parallel for loops
 **/
struct ParallelForData
{
    llvm::CallInst *init;
    llvm::CallInst *fini;
};

/**
 * Struct with pointer to OpenMP Runtime Library calls for reduction pragmas
 **/
struct ReductionData
{
    llvm::CallInst *reduce;
    llvm::CallInst *end_reduce;
};

/**
 * Struct with pointers to OpenMP Runtime Library calls for critical pragmas
 **/
struct CriticalData
{
    llvm::CallInst *critical;
    llvm::CallInst *end_critical;
};

/**
 * This class represent a microtask (omp.outlined) which is created
 * for each OpenMP parallel section.
 **/
class Microtask
{
  private:
    // The __kmpc_fork_call instruction in the original code, which calls the OpenMP microtask
    llvm::CallInst *_fork_call;

    // The microtask function itself (omp.outlined created by the compiler for OpenMP parallel
    // sections).
    llvm::Function *_function;

    // The shared variables used in this Microtask
    std::vector<llvm::Value *> _shared_variables;

    // Parallel for inside the microtask
    std::vector<ParallelForData> _parallel_for;

    // Reduction inside the microtask
    std::vector<ReductionData> _reduction;

    // Critical section inside the microtask
    std::vector<CriticalData> _critical;

  public:
    /**
     * Constructor expects a CallInst* to __kmpc_fork_call
     **/
    Microtask(llvm::CallInst *fork_call);

    ~Microtask();

    llvm::CallInst *get_fork_call();

    llvm::Function *get_function();

    std::vector<ParallelForData> *get_parallel_for();

    std::vector<ReductionData> *get_reductions();

    std::vector<CriticalData> *get_critical();

    bool has_shared_variables();

    std::vector<llvm::Value *> &get_shared_variables();
};

#endif
