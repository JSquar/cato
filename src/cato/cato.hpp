/*
 * File: cato.hpp
 * Created: Tuesday, 27th September 2022 1:42:56 pm
 * Author: Michael Blesel
 * Author: Tim Jammer
 * Author: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 *
 * -----
 * Last Modified: Tuesday, 27th September 2022 2:58:25 pm
 * Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * Copyright (c) 2022 Jannek Squar
 *
 */

#include "MemoryAllocation.h"
#include "Microtask.h"
#include "RuntimeHandler.h"

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassPlugin.h>
#include <vector>

struct CatoPass : public llvm::PassInfoMixin<CatoPass>
{
    bool runOnModule(llvm::Module &M);

    // Force pass usage regardless of IR code built with -O0
    // https://llvm.org/docs/WritingAnLLVMNewPMPass.html#required-passes
    static bool isRequired() { return true; }

    llvm::PreservedAnalyses run(llvm::Module &, llvm::ModuleAnalysisManager &);

    void categorize_memory_access_paths(
        std::vector<std::vector<llvm::Value *>> &paths,
        std::vector<std::pair<int, std::vector<llvm::Value *>>> *store_paths,
        std::vector<std::pair<int, std::vector<llvm::Value *>>> *load_paths,
        std::vector<std::pair<int, std::vector<llvm::Value *>>> *ptr_store_paths,
        std::vector<std::vector<llvm::Value *>> *free_paths);

    template <class T>
    std::vector<llvm::Value *>
    get_memory_access_indices(llvm::Module &M,
                              std::pair<int, std::vector<llvm::Value *>> &categorized_path);

    void replace_sequential_shared_memory_accesses(llvm::Module &M, RuntimeHandler &runtime);

    void replace_sequential_pointers_to_shared_memory(
        llvm::Module &M, RuntimeHandler &runtime,
        std::vector<llvm::StoreInst *> &base_ptr_stores);

    void replace_microtask_shared_memory_accesses(
        llvm::Module &M, RuntimeHandler &runtime,
        std::vector<std::unique_ptr<Microtask>> &microtasks);

    std::vector<std::unique_ptr<Microtask>> find_microtasks(llvm::Module &M);

    std::vector<std::unique_ptr<MemoryAllocation>> find_memory_allocations(llvm::Module &M);

    std::vector<llvm::CallInst *> find_memory_deallocations(llvm::Module &M);

    void replace_fork_calls(llvm::Module &M, RuntimeHandler &runtime,
                            std::vector<std::unique_ptr<Microtask>> &microtasks);

    void replace_memory_allocations(llvm::Module &M, RuntimeHandler &runtime);

    void replace_parallel_for(llvm::Module &M, RuntimeHandler &runtime,
                              std::vector<std::unique_ptr<Microtask>> &microtasks);

    void replace_reductions(llvm::Module &M, RuntimeHandler &runtime,
                            std::vector<std::unique_ptr<Microtask>> &microtasks);

    void replace_criticals(llvm::Module &M, RuntimeHandler &runtime,
                           std::vector<std::unique_ptr<Microtask>> &microtasks);

    void replace_memory_deallocations(llvm::Module &M, RuntimeHandler &runtime);

    void insert_test_func(llvm::Module &M, RuntimeHandler &runtime);
};
