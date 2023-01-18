#include <llvm/Pass.h>

#include <llvm/Transforms/IPO/PassManagerBuilder.h>

#include <llvm/ADT/StringRef.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/AtomicOrdering.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

#include <memory>
#include <set>
// #include <vector>

// #include "Microtask.h"
// #include "RuntimeHandler.h"
#include "UserTree.h"
#include "cato.hpp"
#include "debug.h"
#include "helper.h"

using namespace llvm;

static cl::opt<bool> cato_logging("cato-logging", cl::init(0), cl::Hidden,
                                  cl::desc("Enable CATO logging"));

PreservedAnalyses CatoPass::run(Module &M, ModuleAnalysisManager &)
{
    bool Changed = runOnModule(M);

    return (Changed ? PreservedAnalyses::none() : PreservedAnalyses::all());
}

/**
 * Finds all calls to memory allocation functions like malloc in the IR Module
 * and returns them in form of MemoryAllocation objects.
 **/
std::vector<std::unique_ptr<MemoryAllocation>> CatoPass::find_memory_allocations(Module &M)
{
    std::vector<std::unique_ptr<MemoryAllocation>> memory_allocations;

    // A list of memory allocation functions that get considered
    std::vector<StringRef> allocation_functions = {"malloc", "calloc", "_Znam", "_Znwm"};

    for (auto alloc_func : allocation_functions)
    {
        auto alloc_users = get_function_users(M, alloc_func);

        for (auto &user : alloc_users)
        {
            if (auto *call = dyn_cast<CallInst>(user))
            {
                memory_allocations.push_back(std::make_unique<MemoryAllocation>(call));
            }
        }
    }

    return memory_allocations;
}

/**
 * Finds all calls to memory deallocation functions like free in the IR Module
 * and return them in the form of a CallInst* Vector.
 **/
std::vector<CallInst *> CatoPass::find_memory_deallocations(Module &M)
{
    std::vector<CallInst *> memory_deallocations;

    // TODO more functions
    std::vector<StringRef> deallocation_functions = {"free"};

    for (auto dealloc_func : deallocation_functions)
    {
        auto dealloc_users = get_function_users(M, dealloc_func);

        for (auto &user : dealloc_users)
        {
            if (auto *call = dyn_cast<CallInst>(user))
            {
                memory_deallocations.push_back(call);
            }
        }
    }

    return memory_deallocations;
}

/**
 * Takes a vector of paths from a UserTree and categorizes them after load, store and
 * free instructions on the shared memory object from which the UserTree was created
 *
 * paths: The input vector of paths
 * store_paths: Output parameter for all store paths in paths (only stores of actual
 *values. no pointer stores) load_paths: Output parameter for all load paths in paths (only
 *loads of acrual values. no pointer loads) ptr_store_paths: Output parameter for all paths
 *where a pointer is stored free_paths: Output parameter for all free paths in paths
 **/
void CatoPass::categorize_memory_access_paths(
    std::vector<std::vector<Value *>> &paths,
    std::vector<std::pair<int, std::vector<Value *>>> *store_paths,
    std::vector<std::pair<int, std::vector<Value *>>> *load_paths,
    std::vector<std::pair<int, std::vector<Value *>>> *ptr_store_paths,
    std::vector<std::vector<Value *>> *free_paths)
{
    std::set<Value *> categorized_instructions;

    for (auto &path : paths)
    {
        for (unsigned int i = 0; i < path.size(); i++)
        {
            Value *u = path[i];

            if (auto *store = dyn_cast<StoreInst>(u))
            {
                // This is a store of a non pointer value.
                if (!store->getValueOperand()->getType()->isPointerTy())
                {
                    // Check if the store instruction actually stores to the shared memory
                    // or if the store path just stores the value of the memory abstraction
                    // into something else
                    auto *store_value = store->getValueOperand();
                    if (std::find(path.begin(), path.begin() + i, store_value) !=
                        path.begin() + i)
                    {
                        Debug(errs() << "THIS PATH STORES THE VALUE OF THE MEMORY "
                                        "ABSTRACTION AND IS NOT A STORE TO IT!\n";);
                        Debug(errs() << "    ";);
                        Debug(store->dump(););
                    }
                    // Check if the instructions was already added through a different
                    // path.
                    else if (categorized_instructions.insert(u).second)
                    {
                        store_paths->push_back({i, path});
                    }
                }
                // This is a store of a pointer value
                else
                {
                    Value *store_destination = store->getPointerOperand();
                    if (std::find(path.begin(), path.begin() + i, store_destination) !=
                        path.begin() + i)
                    {
                        Debug(errs() << "Pointer value store to shared memory:\n";);
                        Debug(u->dump(););
                        // Check if the instructions was already added through a different
                        // path.
                        if (categorized_instructions.insert(u).second)
                        {
                            ptr_store_paths->push_back({i, path});
                        }
                    }
                    else
                    {
                        Debug(errs() << "Store dest: ";);
                        Debug(store_destination->dump(););
                        Debug(errs() << "Not a store to memeory abstraction.\n";);
                    }
                }
            }
            else if (auto *load = dyn_cast<LoadInst>(u))
            {
                if (!load->getType()->isPointerTy())
                {
                    // Check if the instructions was already added through a different
                    // path.
                    if (categorized_instructions.insert(u).second)
                    {
                        load_paths->push_back({i, path});
                    }
                }
            }
            else if (auto *call_inst = dyn_cast<CallInst>(u))
            {
                if (call_inst->getCalledFunction()->getName().equals("free"))
                {
                    // Check if the instructions was already added through a different
                    // path.
                    if (categorized_instructions.insert(u).second)
                    {
                        Debug(errs() << "Free call on shared memory:\n";);
                        Debug(call_inst->dump(););
                        free_paths->push_back(path);
                    }
                }
            }
        }
    }
}

/**
 * Analyzes the given pair <index of the store or load instruction in path, the path>
 * and return a vector with the access indices of the load/store instruction.
 *
 * The returned vector is of the form:
 *      1. Number of indices for the memory access
 *      2. A list of the indices (currently a maximum of 3D pointer accesses is supported
 **/
template <class T>
std::vector<Value *>
CatoPass::get_memory_access_indices(Module &M,
                                    std::pair<int, std::vector<Value *>> &categorized_path)
{
    auto &path = categorized_path.second;
    auto *instruction = dyn_cast<T>(path[categorized_path.first]);

    std::vector<Value *> indices;

    IRBuilder<> builder(M.getContext());
    LLVMContext &Ctx = M.getContext();

    // Some debug output
    Debug(errs() << "Analyzing memory access:\n";);
    Debug(instruction->dump(););
    for (int i = categorized_path.first - 1; i >= 0; i--)
    {
        Debug(errs() << " |";);
        Debug(path[i]->dump(););
    }

    Type *ptr_type = nullptr;
    // If the path is from inside a Microtask the first instruction in the path is a load
    // and the type can be derived from that
    if (auto *first_inst = dyn_cast<LoadInst>(path[0]))
    {
        ptr_type = path[0]->getType();
    }
    // If the path is from outside a Microtask we need to find a bitcast to figure out the
    // type
    else
    {
        for (int i = 0; i < categorized_path.first; i++)
        {
            Value *user = path[i];
            if (auto *cast = dyn_cast<BitCastInst>(user))
            {
                ptr_type = cast->getType();
            }
        }
    }

    if (ptr_type == nullptr)
    {
        // ptr_type = path[0]->getType();
        errs() << "Error: could not find out type of memory access\n";
    }

    int ptr_depth = get_pointer_depth(ptr_type);

    builder.SetInsertPoint(instruction);

    // Check for all known access patterns to 1D/2D arrays/pointers
    // and determine the offsets for the memory access
    if (ptr_depth == 1)
    {
        if (auto *gep = dyn_cast<GetElementPtrInst>(instruction->getPointerOperand()))
        {
            // TODO fix this case
            if (auto *gep2 = dyn_cast<GetElementPtrInst>(gep->getOperand(0)))
            {
                errs() << "Error: Unknown IR Pattern for evaluation of accessing index "
                          "for 1d array.\n";
                gep2->dump();
            }

            Value *index = gep->getOperand(1);
            Debug(errs() << "Pointer access at index:\n";);
            Debug(errs() << "  ->index1: ";);
            Debug(index->dump(););

            indices.push_back(builder.getInt32(1));
            indices.push_back(index);
        }
        else
        {
            Debug(errs() << "Pointer access at index:\n";);
            Debug(errs() << "  ->index1: 0\n";);

            indices.push_back(builder.getInt32(1));
            indices.push_back(builder.getInt64(0));
        }
    }
    else if (ptr_depth == 2)
    {
        Debug(errs() << "Pointer depth of 2\n";);
        if (auto *last_inst = dyn_cast<LoadInst>(instruction->getPointerOperand()))
        {
            if (auto *gep = dyn_cast<GetElementPtrInst>(last_inst->getPointerOperand()))
            {
                Value *index = gep->getOperand(1);
                Debug(errs() << "Pointer access at index:\n";);
                Debug(errs() << "  ->index1: ";);
                Debug(index->dump(););
                Debug(errs() << "  ->index2: 0\n";);

                indices.push_back(builder.getInt32(2));
                indices.push_back(index);
                indices.push_back(builder.getInt64(0));
            }
            else
            {
                Debug(errs() << "Pointer access at index:\n";);
                Debug(errs() << "  ->index1: 0\n";);
                Debug(errs() << "  ->index2: 0\n";);

                indices.push_back(builder.getInt32(2));
                indices.push_back(builder.getInt64(0));
                indices.push_back(builder.getInt64(0));
            }
        }
        else if (auto *gep2 = dyn_cast<GetElementPtrInst>(instruction->getPointerOperand()))
        {
            if (auto *inst = dyn_cast<LoadInst>(gep2->getPointerOperand()))
            {
                if (auto *gep1 = dyn_cast<GetElementPtrInst>(inst->getPointerOperand()))
                {
                    Value *index1 = gep1->getOperand(1);
                    Value *index2 = gep2->getOperand(1);
                    Debug(errs() << "Pointer access at index:\n";);
                    Debug(errs() << "  ->index1: ";);
                    Debug(index1->dump(););
                    Debug(errs() << "  ->index2: ";);
                    Debug(index2->dump(););

                    indices.push_back(builder.getInt32(2));
                    indices.push_back(index1);
                    indices.push_back(index2);
                }
                else
                {
                    Value *index2 = gep2->getOperand(1);
                    Debug(errs() << "Pointer access at index:\n";);
                    Debug(errs() << "  ->index1: 0\n";);
                    Debug(errs() << "  ->index2: ";);
                    Debug(index2->dump(););

                    indices.push_back(builder.getInt32(2));
                    indices.push_back(builder.getInt64(0));
                    indices.push_back(index2);
                }
            }
            else
            {
                errs() << "Unrecognized pointer access pattern!\n";
            }
        }
        else
        {
            errs() << "Unrecognized pointer access pattern!\n";
        }
    }
    else if (ptr_depth == 3)
    {
        Debug(errs() << "Pointer depth of 3\n";);

        if (auto *load = dyn_cast<LoadInst>(instruction->getPointerOperand()))
        {
            if (auto *load1 = dyn_cast<LoadInst>(load->getPointerOperand()))
            {
                if (auto *gep2 = dyn_cast<GetElementPtrInst>(load1->getPointerOperand()))
                {
                    indices.push_back(builder.getInt32(3));
                    indices.push_back(gep2->getOperand(1));
                    indices.push_back(builder.getInt64(0));
                    indices.push_back(builder.getInt64(0));
                }
                else
                {
                    indices.push_back(builder.getInt32(3));
                    indices.push_back(builder.getInt64(0));
                    indices.push_back(builder.getInt64(0));
                    indices.push_back(builder.getInt64(0));
                }
            }
            else if (auto *gep1 = dyn_cast<GetElementPtrInst>(load->getPointerOperand()))
            {
                if (auto *load2 = dyn_cast<LoadInst>(gep1->getPointerOperand()))
                {
                    if (auto *gep2 = dyn_cast<GetElementPtrInst>(load2->getPointerOperand()))
                    {
                        indices.push_back(builder.getInt32(3));
                        indices.push_back(gep2->getOperand(1));
                        indices.push_back(gep1->getOperand(1));
                        indices.push_back(builder.getInt64(0));
                    }
                    else
                    {
                        indices.push_back(builder.getInt32(3));
                        indices.push_back(builder.getInt64(0));
                        indices.push_back(gep1->getOperand(1));
                        indices.push_back(builder.getInt64(0));
                    }
                }
                else
                {
                    errs() << "Error in 3d index calculation\n";
                }
            }
        }
        else if (auto *gep = dyn_cast<GetElementPtrInst>(instruction->getPointerOperand()))
        {
            if (auto *load1 = dyn_cast<LoadInst>(gep->getPointerOperand()))
            {
                if (auto *load2 = dyn_cast<LoadInst>(load1->getPointerOperand()))
                {
                    if (auto *gep2 = dyn_cast<GetElementPtrInst>(load2->getPointerOperand()))
                    {
                        indices.push_back(builder.getInt32(3));
                        indices.push_back(gep2->getOperand(1));
                        indices.push_back(builder.getInt64(0));
                        indices.push_back(gep->getOperand(1));
                    }
                    else
                    {
                        indices.push_back(builder.getInt32(3));
                        indices.push_back(builder.getInt64(0));
                        indices.push_back(builder.getInt64(0));
                        indices.push_back(gep->getOperand(1));
                    }
                }
                else if (auto *gep2 = dyn_cast<GetElementPtrInst>(load1->getPointerOperand()))
                {
                    if (auto *load2 = dyn_cast<LoadInst>(gep2->getPointerOperand()))
                    {
                        if (auto *gep3 =
                                dyn_cast<GetElementPtrInst>(load2->getPointerOperand()))
                        {
                            indices.push_back(builder.getInt32(3));
                            indices.push_back(gep3->getOperand(1));
                            indices.push_back(gep2->getOperand(1));
                            indices.push_back(gep->getOperand(1));
                        }
                        else
                        {
                            indices.push_back(builder.getInt32(3));
                            indices.push_back(builder.getInt64(0));
                            indices.push_back(gep2->getOperand(1));
                            indices.push_back(gep->getOperand(1));
                        }
                    }
                    else
                    {
                        errs() << "Error in 3d index calculation\n";
                    }
                }
            }
            else
            {
                errs() << "Error in 3d index calculation\n";
            }
        }

        Debug(errs() << "Pointer access at index:\n";);
        Debug(errs() << "  -> index1: ";);
        Debug(indices[1]->dump(););
        Debug(errs() << "  -> index2: ";);
        Debug(indices[2]->dump(););
        Debug(errs() << "  -> index3: ";);
        Debug(indices[3]->dump(););
    }
    else
    {
        errs() << "Error: Pointer depth > 3 not supported at the moment!\n";
    }

    return indices;
}

/**
 * Looks at all load, store and free instructions that are used on shared memory segments
 * in non OpenMP sections of the original program.
 * The memory allocations for the shared memory segments have to already be done through
 * the runtime library call 'allocate_shared_memory'.
 *
 * The functions creates UserTrees for each shared memory allocation and looks
 * for store/load instructions on non pointer values (also free memory calls) to the shared
 * memory object and replaces them by corresponding calls to the cato runtime library.
 *
 * Accesses to shared memory inside Microtasks are not handled by this function
 **/
void CatoPass::replace_sequential_shared_memory_accesses(Module &M, RuntimeHandler &runtime)
{
    std::vector<std::vector<Value *>> paths;

    // For each 'allocate_shared_memory' function call a UserTree gets created
    // and all paths from those trees are collected into 'paths'
    for (auto *allocate_call : runtime.functions.allocate_shared_memory->users())
    {
        UserTree T(allocate_call);
        auto tmp_paths = T.get_all_paths();
        paths.insert(paths.end(), tmp_paths.begin(), tmp_paths.end());
    }

    // Find pointer stores into structs
    // TODO clean up and move to own function probably
    std::vector<Value *> struct_geps;
    int path_count_before = paths.size();
    for (int j = 0; j < path_count_before; j++)
    {
        auto &path = paths[j];
        for (unsigned int i = 0; i < path.size(); i++)
        {
            Value *u = path[i];

            if (auto *store = dyn_cast<StoreInst>(u))
            {
                if (store->getValueOperand()->getType()->isPointerTy())
                {
                    Value *store_destination = store->getPointerOperand();

                    if (auto *gep = dyn_cast<GetElementPtrInst>(store_destination))
                    {
                        if (gep->getPointerOperandType()
                                ->getPointerElementType()
                                ->isStructTy())
                        {
                            errs() << "FOUND STORE INTO STRUCT:\n";
                            errs() << "    ";
                            gep->getPointerOperand()->dump();

                            for (auto *user : gep->getPointerOperand()->users())
                            {
                                if (auto *gep2 = dyn_cast<GetElementPtrInst>(user))
                                {
                                    if (gep2->getOperand(1) == gep->getOperand(1) &&
                                        gep2->getOperand(2) == gep->getOperand(2))
                                    {
                                        for (auto *user : gep2->users())
                                        {
                                            if (auto *load = dyn_cast<LoadInst>(user))
                                            {
                                                UserTree T(load);
                                                auto tmp_paths = T.get_all_paths();
                                                paths.insert(paths.end(), tmp_paths.begin(),
                                                             tmp_paths.end());
                                                for (auto &p : tmp_paths)
                                                {
                                                    errs() << "STRUCT PATH:\n";
                                                    for (auto *e : p)
                                                    {
                                                        e->dump();
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Find pointer stores where the base pointer to the memory abstraction is stored in
    // another pointer This is for example the case when a memory abstraction is passed to
    // a Microtask function.
    // TODO probably add struct handling here
    std::vector<StoreInst *> base_pointer_stores;
    for (auto &path : paths)
    {
        for (unsigned int i = 0; i < path.size(); i++)
        {
            Value *u = path[i];

            if (auto *store = dyn_cast<StoreInst>(u))
            {
                if (store->getValueOperand()->getType()->isPointerTy())
                {
                    Value *store_destination = store->getPointerOperand();
                    if (std::find(path.begin(), path.begin() + i, store_destination) ==
                        path.begin() + i)
                    {
                        // Case: The shared memory abstraction is stored in a
                        // local pointer. This is the case if it is later
                        // passed to a ompoutlined function
                        if (auto *alloca = dyn_cast<AllocaInst>(store_destination))
                        {
                            base_pointer_stores.push_back(store);
                        }
                    }
                }
            }
        }
    }

    replace_sequential_pointers_to_shared_memory(M, runtime, base_pointer_stores);

    // Print out all paths for debugging purposes
    for (auto &path : paths)
    {
        Debug(errs() << "PATH:\n";);
        for (auto &u : path)
        {
            Debug(u->dump(););
        }
        Debug(errs() << "PATH END\n";);
    }
    Debug(errs() << "========================================\n";);

    // The paths need to be categorized into paths that lead to store, load or free
    // instructions Only Accesses to non pointer values are categorized. The manipulation
    // of pointer values in shared memory objects is not supported at the moment.

    // The pair consists of: <Index of the load/store instruction in path, the path>
    std::vector<std::pair<int, std::vector<Value *>>> store_paths, load_paths, ptr_store_paths;
    std::vector<std::vector<Value *>> free_paths;

    categorize_memory_access_paths(paths, &store_paths, &load_paths, &ptr_store_paths,
                                   &free_paths);

    IRBuilder<> builder(M.getContext());
    LLVMContext &Ctx = M.getContext();
    // Now we need to find the offsets of the shared memory accesses
    // Currently only 1D and 2D arrays/pointers are supported
    for (auto &p : load_paths)
    {
        auto &path = p.second;
        auto *load = dyn_cast<LoadInst>(path[p.first]);

        // Get a vector of the number of indices followed by the indices for
        // the memory access on the shared memory object
        std::vector<Value *> args = get_memory_access_indices<LoadInst>(M, p);

        // Find the correct value for the base pointer.
        // If the allocate call is not in the same function as the load
        // the base pointer value has to be the argument value from the current function.
        Function *current_func = load->getFunction();
        if (dyn_cast<Instruction>(path[0])->getFunction() != current_func)
        {
            for (int i = p.first; i > 0; i--)
            {
                if (auto *call_inst = dyn_cast<CallInst>(path[i]))
                {
                    // Identify the argument argument of the current function that is
                    // the shared memory object
                    auto *last_inst = path[i - 1];
                    Value *matching_argument = nullptr;

                    for (unsigned int j = 0; j < call_inst->arg_size(); j++)
                    {
                        if (last_inst == call_inst->getArgOperand(j))
                        {
                            Function *called_function = call_inst->getCalledFunction();
                            if (called_function->arg_begin() + j < called_function->arg_end())
                            {
                                matching_argument = called_function->arg_begin() + j;
                            }
                        }
                    }

                    builder.SetInsertPoint(load);
                    Value *void_ptr =
                        builder.CreateBitCast(matching_argument, Type::getInt8PtrTy(Ctx));
                    args.insert(args.begin(), void_ptr);
                    break;
                }
            }
        }
        else if (!path[0]->getType()->isVoidTy())
        {
            builder.SetInsertPoint(load);
            Value *void_ptr = builder.CreateBitCast(path[0], Type::getInt8PtrTy(Ctx));
            args.insert(args.begin(), void_ptr);
        }
        else
        {
            args.insert(args.begin(), path[0]);
        }

        // Do the actual replacement of the load instruction with a call to the CATO
        // runtime library
        if (args.size() >= 3)
        {
            builder.SetInsertPoint(load->getFunction()->getEntryBlock().getFirstNonPHI());
            Value *load_value = builder.CreateAlloca(load->getType());
            builder.SetInsertPoint(load);
            Value *void_ptr = builder.CreateBitCast(load_value, Type::getInt8PtrTy(Ctx));
            args.insert(args.begin() + 1, void_ptr);
            CallInst *load_call =
                builder.CreateCall(runtime.functions.shared_memory_sequential_load, args);
            Value *bitcast = builder.CreateBitCast(void_ptr, load->getPointerOperandType());
            Value *new_load = builder.CreateLoad(bitcast->getType()->getPointerElementType(),
                                                 bitcast, "CATO: New Load Call");
            load->replaceAllUsesWith(new_load);
            load->eraseFromParent();
        }
    }

    // Now we need to find the offsets of the shared memory accesses
    // Currently only 1D and 2D arrays/pointers are supported
    for (auto &p : store_paths)
    {
        auto &path = p.second;
        auto *store = dyn_cast<StoreInst>(path[p.first]);

        // Get a vector of the number of indices followed by the indices for
        // the memory access on the shared memory object
        std::vector<Value *> args = get_memory_access_indices<StoreInst>(M, p);

        // Find the correct value for the base pointer.
        // If the allocate call is not in the same function as the load
        // the base pointer value has to be the argument value from the current function.
        Function *current_func = store->getFunction();
        if (dyn_cast<Instruction>(path[0])->getFunction() != current_func)
        {
            for (int i = p.first; i > 0; i--)
            {
                if (auto *call_inst = dyn_cast<CallInst>(path[i]))
                {
                    // Identify the argument argument of the current function that is
                    // the shared memory object
                    auto *last_inst = path[i - 1];
                    Value *matching_argument = nullptr;

                    for (unsigned int j = 0; j < call_inst->arg_size(); j++)
                    {
                        if (last_inst == call_inst->getArgOperand(j))
                        {
                            Function *called_function = call_inst->getCalledFunction();
                            if (called_function->arg_begin() + j < called_function->arg_end())
                            {
                                matching_argument = called_function->arg_begin() + j;
                            }
                        }
                    }

                    builder.SetInsertPoint(store);
                    Value *void_ptr =
                        builder.CreateBitCast(matching_argument, Type::getInt8PtrTy(Ctx));
                    args.insert(args.begin(), void_ptr);
                    break;
                }
            }
        }
        else if (!path[0]->getType()->isVoidTy())
        {
            builder.SetInsertPoint(store);
            Value *void_ptr = builder.CreateBitCast(path[0], Type::getInt8PtrTy(Ctx));
            args.insert(args.begin(), void_ptr);
        }
        else
        {
            args.insert(args.begin(), path[0]);
        }

        // Do the acutal replacement of the load instruction with a call to the cato
        // runtime library
        if (args.size() >= 3)
        {
            // TODO only allocate a new value if the original store had a constant value
            // that was stored
            builder.SetInsertPoint(store->getFunction()->getEntryBlock().getFirstNonPHI());
            Value *store_value = builder.CreateAlloca(store->getValueOperand()->getType());
            builder.SetInsertPoint(store);
            builder.CreateStore(store->getOperand(0), store_value);
            Value *void_ptr = builder.CreateBitCast(store_value, Type::getInt8PtrTy(Ctx));
            args.insert(args.begin() + 1, void_ptr);
            Value *new_store_call =
                builder.CreateCall(runtime.functions.shared_memory_sequential_store, args);
            store->replaceAllUsesWith(new_store_call);
            store->eraseFromParent();
        }
    }

    for (auto &p : ptr_store_paths)
    {
        auto &path = p.second;
        auto *store = dyn_cast<StoreInst>(path[p.first]);

        Debug(errs() << "Pointer store Path:\n";);
        for (auto &x : path)
        {
            Debug(x->dump(););
        }

        // TODO
        // if (get_pointer_depth(store->getValueOperand()->getType()) < 2)
        if (get_pointer_depth(store->getValueOperand()->getType()) < 3)
        {
            builder.SetInsertPoint(store);

            Value *index = nullptr;

            if (auto *gep = dyn_cast<GetElementPtrInst>(store->getPointerOperand()))
            {
                index = gep->getOperand(1);
            }
            else
            {
                index = builder.getInt64(0);
            }

            Value *dest_ptr =
                builder.CreateBitCast(store->getPointerOperand(), Type::getInt8PtrTy(Ctx));
            Value *source_ptr =
                builder.CreateBitCast(store->getValueOperand(), Type::getInt8PtrTy(Ctx));
            std::vector<Value *> args = {dest_ptr, source_ptr, index};

            auto *new_call =
                builder.CreateCall(runtime.functions.shared_memory_pointer_store, args);
            store->replaceAllUsesWith(new_call);
            store->eraseFromParent();
        }
    }

    // Replace freeing of shared memory with corresponding call to the cato runtime library
    for (auto &path : free_paths)
    {
        if (auto *free_call = dyn_cast<CallInst>(path.back()))
        {
            Debug(errs() << "Analyzing free call instruction:\n";);
            Debug(free_call->dump(););

            for (int i = path.size() - 2; i >= 0; i--)
            {
                Debug(errs() << " |";);
                Debug(path[i]->dump(););
            }

            builder.SetInsertPoint(free_call);
            auto new_free_call = builder.CreateCall(runtime.functions.shared_memory_free,
                                                    free_call->getOperand(0));
            free_call->replaceAllUsesWith(new_free_call);
            free_call->eraseFromParent();
        }
        else
        {
            errs() << "Error: free path ends in non call instruction\n";
        }
    }
}

/**
 * In some cases the pointer to a shared memory object gets stored in another variable
 * and the shared memory is accessed through the new pointer.
 * This happens for shared variable that are passed to a Microtask. In that case the
 * compiler creates a pointer to the pointer for the shared memory and passes it as
 * argument to the Microtask function.
 * To ensure that all accesses to the shared memory in the non-OpenMP sections of the code
 * are handled correctly by the CATO Runtime Library these stores of the base address of a
 * shared memory variable have to be found and all loads/stores to the shared memory
 * have to be replaced by calls to the CATO Runtime Library.
 **/
void CatoPass::replace_sequential_pointers_to_shared_memory(
    Module &M, RuntimeHandler &runtime, std::vector<StoreInst *> &base_ptr_stores)
{
    std::vector<Value *> pointers_to_shared_memory;

    for (auto &ptr_store : base_ptr_stores)
    {
        Debug(errs() << "BASE_PTR_STORE: ";);
        Debug(ptr_store->dump(););
        Debug(errs() << "-->";);
        Debug(ptr_store->getPointerOperand()->dump(););
        pointers_to_shared_memory.push_back(ptr_store->getPointerOperand());
    }

    std::vector<std::vector<Value *>> paths;

    for (auto &value : pointers_to_shared_memory)
    {
        for (auto *user : value->users())
        {
            if (auto *load = dyn_cast<LoadInst>(user))
            {
                UserTree T(load);
                auto tmp_paths = T.get_all_paths();
                paths.insert(paths.end(), tmp_paths.begin(), tmp_paths.end());
            }
        }
    }

    Debug(errs() << "POINTER TO SHARED MEMORY PATHS: \n";);
    for (auto &path : paths)
    {
        Debug(errs() << "PATH BEGIN\n";);
        for (auto &u : path)
        {
            Debug(errs() << "    ");
            Debug(u->dump(););
        }
        Debug(errs() << "PATH END\n";);
    }

    std::vector<std::pair<int, std::vector<Value *>>> store_paths;
    std::vector<std::pair<int, std::vector<Value *>>> load_paths;
    std::set<Value *> categorized_instructions;

    for (auto &path : paths)
    {
        for (unsigned int i = 0; i < path.size(); i++)
        {
            Value *u = path[i];

            if (auto *store = dyn_cast<StoreInst>(u))
            {
                if (!store->getValueOperand()->getType()->isPointerTy())
                {
                    // Check if the instructions was already added through a different
                    // path.
                    if (categorized_instructions.insert(u).second)
                    {
                        store_paths.push_back({i, path});
                    }
                }
            }
            else if (auto *load = dyn_cast<LoadInst>(u))
            {
                if (!load->getType()->isPointerTy())
                {
                    // Check if the instructions was already added through a different
                    // path.
                    if (categorized_instructions.insert(u).second)
                    {
                        load_paths.push_back({i, path});
                    }
                }
            }
        }
    }

    IRBuilder<> builder(M.getContext());
    LLVMContext &Ctx = M.getContext();

    for (auto &p : load_paths)
    {
        auto &path = p.second;
        auto *load = dyn_cast<LoadInst>(path[p.first]);

        std::vector<Value *> args = get_memory_access_indices<LoadInst>(M, p);

        // Find the correct value for the base pointer.
        // If the allocate call is not in the same function as the load
        // the base pointer value has to be the argument value from the current function.
        Function *current_func = load->getFunction();
        if (dyn_cast<Instruction>(path[0])->getFunction() != current_func)
        {
            for (int i = p.first; i > 0; i--)
            {
                if (auto *call_inst = dyn_cast<CallInst>(path[i]))
                {
                    errs() << "   CallInst: ";
                    call_inst->dump();

                    // Identify the argument argument of the current function that is
                    // the shared memory object
                    auto *last_inst = path[i - 1];
                    Value *matching_argument = nullptr;

                    for (unsigned int j = 0; j < call_inst->arg_size(); j++)
                    {
                        if (last_inst == call_inst->getArgOperand(j))
                        {
                            Function *called_function = call_inst->getCalledFunction();
                            if (called_function->arg_begin() + j < called_function->arg_end())
                            {
                                matching_argument = called_function->arg_begin() + j;
                            }
                        }
                    }

                    builder.SetInsertPoint(load);
                    Value *void_ptr =
                        builder.CreateBitCast(matching_argument, Type::getInt8PtrTy(Ctx));
                    args.insert(args.begin(), void_ptr);
                    break;
                }
            }
        }
        else
        {
            builder.SetInsertPoint(load);
            Value *void_base_ptr = builder.CreateBitCast(path[0], Type::getInt8PtrTy(Ctx));

            args.insert(args.begin(), void_base_ptr);
        }

        // Do the actutal replacement of the load instruction with a call to the cato
        // runtime library
        if (args.size() >= 3)
        {
            builder.SetInsertPoint(load->getFunction()->getEntryBlock().getFirstNonPHI());
            Value *load_value = builder.CreateAlloca(load->getType());
            builder.SetInsertPoint(load);
            Value *void_ptr = builder.CreateBitCast(load_value, Type::getInt8PtrTy(Ctx));
            args.insert(args.begin() + 1, void_ptr);
            auto *load_call =
                builder.CreateCall(runtime.functions.shared_memory_sequential_load, args);
            Value *bitcast = builder.CreateBitCast(void_ptr, load->getPointerOperandType());
            Value *new_load = builder.CreateLoad(bitcast->getType()->getPointerElementType(),
                                                 bitcast, "CATO: Replacement of load call");
            load->replaceAllUsesWith(new_load);
            load->eraseFromParent();
        }
    }

    for (auto &p : store_paths)
    {
        auto &path = p.second;
        auto *store = dyn_cast<StoreInst>(path[p.first]);

        // Get a vector of the number of indices followed by the indices for
        // the memory access on the shared memory object
        std::vector<Value *> args = get_memory_access_indices<StoreInst>(M, p);

        // Find the correct value for the base pointer.
        // If the allocate call is not in the same function as the load
        // the base pointer value has to be the argument value from the current function.
        Function *current_func = store->getFunction();
        if (dyn_cast<Instruction>(path[0])->getFunction() != current_func)
        {
            for (int i = p.first; i > 0; i--)
            {
                if (auto *call_inst = dyn_cast<CallInst>(path[i]))
                {
                    errs() << "   CallInst: ";
                    call_inst->dump();

                    // Identify the argument argument of the current function that is
                    // the shared memory object
                    auto *last_inst = path[i - 1];
                    Value *matching_argument = nullptr;

                    for (unsigned int j = 0; j < call_inst->arg_size(); j++)
                    {
                        if (last_inst == call_inst->getArgOperand(j))
                        {
                            Function *called_function = call_inst->getCalledFunction();
                            if (called_function->arg_begin() + j < called_function->arg_end())
                            {
                                matching_argument = called_function->arg_begin() + j;
                            }
                        }
                    }

                    builder.SetInsertPoint(store);
                    Value *void_ptr =
                        builder.CreateBitCast(matching_argument, Type::getInt8PtrTy(Ctx));
                    args.insert(args.begin(), void_ptr);
                    break;
                }
            }
        }
        else
        {
            builder.SetInsertPoint(store);
            Value *void_base_ptr = builder.CreateBitCast(path[0], Type::getInt8PtrTy(Ctx));

            args.insert(args.begin(), void_base_ptr);
        }

        // Do the acutal replacement of the load instruction with a call to the cato
        // runtime library
        if (args.size() >= 3)
        {
            // TODO only allocate a new value if the original store had a constant value
            // that was stored
            builder.SetInsertPoint(store->getFunction()->getEntryBlock().getFirstNonPHI());
            Value *store_value = builder.CreateAlloca(store->getValueOperand()->getType());
            builder.SetInsertPoint(store);
            builder.CreateStore(store->getOperand(0), store_value);
            Value *void_ptr = builder.CreateBitCast(store_value, Type::getInt8PtrTy(Ctx));
            args.insert(args.begin() + 1, void_ptr);
            Value *new_store_call =
                builder.CreateCall(runtime.functions.shared_memory_sequential_store, args);
            store->replaceAllUsesWith(new_store_call);
            store->eraseFromParent();
        }
    }
}

/**
 * Looks at all load, store and free instructions that are used on shared memory segments
 * in Microtasks
 * The memory allocations for the shared memory segments have to already be done through
 * the runtime library call 'allocate_shared_memory'.
 *
 * The functions creates UserTrees for each shared memory allocation and looks
 * for store/load instructions on non pointer values (also free memory calls) to the shared
 * memory object and replaces them by corresponding calls to the cato runtime library.
 *
 * Accesses to shared memory outside of Microtasks are not handled by this function.
 **/
void CatoPass::replace_microtask_shared_memory_accesses(
    Module &M, RuntimeHandler &runtime, std::vector<std::unique_ptr<Microtask>> &microtasks)
{
    for (auto &microtask : microtasks)
    {
        std::vector<Value *> shared_value_variables, shared_pointer_variables,
            shared_struct_variables;

        for (auto &shared_variable : microtask->get_shared_variables())
        {
            if (get_pointer_depth(shared_variable) == 1)
            {
                // Special handling for struct shared variables needed
                if (shared_variable->getType()->getPointerElementType()->isStructTy())
                {
                    shared_struct_variables.push_back(shared_variable);
                    Debug(errs() << "Shared struct variable in Microtask: ";);
                    Debug(shared_variable->dump(););
                }
                else
                {
                    shared_value_variables.push_back(shared_variable);
                    Debug(errs() << "Shared value variable in Microtask: ";);
                    Debug(shared_variable->dump(););
                }
            }
            else
            {
                shared_pointer_variables.push_back(shared_variable);
                Debug(errs() << "Shared pointer variable in Microtask: ";);
                Debug(shared_variable->dump(););
            }
        }

        // Check if the accessed struct member is a single value or pointer variable
        for (auto *struct_var : shared_struct_variables)
        {
            for (auto *user : struct_var->users())
            {
                if (get_pointer_depth(user->getType()) == 1)
                {
                    shared_value_variables.push_back(user);
                }
                else if (get_pointer_depth(user->getType()) > 1)
                {
                    shared_pointer_variables.push_back(user);
                }
            }
        }

        for (auto &single_value_var : shared_value_variables)
        {
            Debug(errs() << "Analysing single value shared variable: ";);
            Debug(single_value_var->dump(););

            // Check for stores to the shared value.
            // If it is not modified in the microtask we don't need
            // to create a memory abstraction for it.
            bool has_stores = false;
            for (auto *user : single_value_var->users())
            {
                if (auto *store = dyn_cast<StoreInst>(user))
                {
                    has_stores = true;
                    break;
                }
            }

            if (has_stores)
            {
                IRBuilder<> builder(M.getContext());
                LLVMContext &Ctx = M.getContext();
                builder.SetInsertPoint(
                    microtask->get_function()->getEntryBlock().getFirstNonPHI());

                Value *void_ptr = nullptr;
                // If this is a GEP that means we are working with a struct
                // and the base pointer to the single value shared var has to
                // be declared at the beginning of the function
                // for the mpi window to be created
                if (auto *gep = dyn_cast<GetElementPtrInst>(single_value_var))
                {
                    auto *new_gep =
                        builder.CreateGEP(nullptr, gep->getPointerOperand(),
                                          {gep->getOperand(1), gep->getOperand(2)});
                    void_ptr = builder.CreateBitCast(new_gep, Type::getInt8PtrTy(Ctx));
                }
                else
                {
                    void_ptr =
                        builder.CreateBitCast(single_value_var, Type::getInt8PtrTy(Ctx));
                }
                Type *type = single_value_var->getType();

                std::vector<Value *> args = {void_ptr,
                                             builder.getInt32(get_mpi_datatype(type))};

                builder.CreateCall(runtime.functions.allocate_shared_value, args);

                // Find all store and load instructions on the shared value

                Debug(errs() << "Users for shared value:\n";);
                for (auto *user : single_value_var->users())
                {
                    if (auto *store = dyn_cast<StoreInst>(user))
                    {
                        Debug(errs() << "Replacing store instruction for shared value: ";);
                        Debug(store->dump(););

                        builder.SetInsertPoint(
                            store->getFunction()->getEntryBlock().getFirstNonPHI());
                        Value *store_value =
                            builder.CreateAlloca(store->getValueOperand()->getType());
                        builder.SetInsertPoint(store);
                        builder.CreateStore(store->getOperand(0), store_value);
                        Value *store_value_ptr =
                            builder.CreateBitCast(store_value, Type::getInt8PtrTy(Ctx));

                        std::vector<Value *> args = {void_ptr, store_value_ptr};

                        Value *new_store_call =
                            builder.CreateCall(runtime.functions.shared_value_store, args);
                        store->replaceAllUsesWith(new_store_call);
                        store->eraseFromParent();
                    }
                    else if (auto *load = dyn_cast<LoadInst>(user))
                    {
                        Debug(errs() << "Replacing load instruction for shared value: ";);
                        Debug(load->dump(););

                        std::vector<Value *> args = {void_ptr, void_ptr};

                        builder.SetInsertPoint(load);
                        builder.CreateCall(runtime.functions.shared_value_load, args);
                    }
                }

                // synchronize shared value at the end of the microtask
                auto return_instructions =
                    get_instruction_in_function<ReturnInst>(microtask->get_function());
                for (auto &ret : return_instructions)
                {
                    builder.SetInsertPoint(ret);
                    builder.CreateCall(runtime.functions.shared_value_synchronize, void_ptr);
                }
            }
        }

        std::vector<std::vector<Value *>> paths;
        for (auto *pointer_var : shared_pointer_variables)
        {
            for (auto *user : pointer_var->users())
            {
                Debug(errs() << "USER: ";);
                Debug(user->dump(););
                UserTree T(user);
                auto tmp_paths = T.get_all_paths();
                paths.insert(paths.end(), tmp_paths.begin(), tmp_paths.end());
            }
        }

        for (auto &path : paths)
        {
            Debug(errs() << "MICROTASK PATH:\n";);
            for (auto &u : path)
            {
                Debug(u->dump(););
            }
            Debug(errs() << "MICROTASK PATH END\n";);
        }

        Debug(errs() << "========================================\n";);

        std::vector<std::pair<int, std::vector<Value *>>> store_paths, load_paths,
            ptr_store_paths;
        std::vector<std::vector<Value *>> free_paths;

        categorize_memory_access_paths(paths, &store_paths, &load_paths, &ptr_store_paths,
                                       &free_paths);

        IRBuilder<> builder(M.getContext());
        LLVMContext &Ctx = M.getContext();
        // Now we need to find the offsets of the shared memory accesses
        // Currently only 1D and 2D arrays/pointers are supported
        for (auto &p : load_paths)
        {
            auto &path = p.second;
            auto *load = dyn_cast<LoadInst>(path[p.first]);

            // Get a vector of the number of indices followed by the indices for
            // the memory access on the shared memory object
            std::vector<Value *> args = get_memory_access_indices<LoadInst>(M, p);

            // Find the correct value for the base pointer.
            // If the allocate call is not in the same function as the load
            // the base pointer value has to be the argument value from the current
            // function.
            Function *current_func = load->getFunction();
            if (dyn_cast<Instruction>(path[0])->getFunction() != current_func)
            {
                for (int i = p.first; i > 0; i--)
                {
                    if (auto *call_inst = dyn_cast<CallInst>(path[i]))
                    {
                        errs() << "   CallInst: ";
                        call_inst->dump();

                        // Identify the argument argument of the current function that is
                        // the shared memory object
                        auto *last_inst = path[i - 1];
                        Value *matching_argument = nullptr;

                        for (unsigned int j = 0; j < call_inst->arg_size(); j++)
                        {
                            if (last_inst == call_inst->getArgOperand(j))
                            {
                                Function *called_function = call_inst->getCalledFunction();
                                if (called_function->arg_begin() + j <
                                    called_function->arg_end())
                                {
                                    matching_argument = called_function->arg_begin() + j;
                                }
                            }
                        }

                        builder.SetInsertPoint(load);
                        Value *void_ptr =
                            builder.CreateBitCast(matching_argument, Type::getInt8PtrTy(Ctx));
                        args.insert(args.begin(), void_ptr);
                        break;
                    }
                }
            }
            else
            {
                builder.SetInsertPoint(load);
                Value *void_ptr = builder.CreateBitCast(path[0], Type::getInt8PtrTy(Ctx));
                args.insert(args.begin(), void_ptr);
            }

            // Do the actual replacement of the load instruction with a call to the cato
            // runtime library
            if (args.size() >= 3)
            {
                builder.SetInsertPoint(load->getFunction()->getEntryBlock().getFirstNonPHI());
                Value *load_value = builder.CreateAlloca(load->getType());
                builder.SetInsertPoint(load);
                Value *void_ptr = builder.CreateBitCast(load_value, Type::getInt8PtrTy(Ctx));
                args.insert(args.begin() + 1, void_ptr);
                CallInst *load_call =
                    builder.CreateCall(runtime.functions.shared_memory_load, args);
                Value *bitcast =
                    builder.CreateBitCast(void_ptr, load->getPointerOperandType());
                LoadInst *new_load = builder.CreateLoad(bitcast->getType(), bitcast);
                load->replaceAllUsesWith(new_load);
                load->eraseFromParent();
            }
        }

        // Now we need to find the offsets of the shared memory accesses
        // Currently only 1D and 2D arrays/pointers are supported
        for (auto &p : store_paths)
        {
            auto &path = p.second;
            auto *store = dyn_cast<StoreInst>(path[p.first]);

            // Get a vector of the number of indices followed by the indices for
            // the memory access on the shared memory object
            std::vector<Value *> args = get_memory_access_indices<StoreInst>(M, p);

            // Find the correct value for the base pointer.
            // If the allocate call is not in the same function as the load
            // the base pointer value has to be the argument value from the current
            // function.
            Function *current_func = store->getFunction();
            if (dyn_cast<Instruction>(path[0])->getFunction() != current_func)
            {
                for (int i = p.first; i > 0; i--)
                {
                    if (auto *call_inst = dyn_cast<CallInst>(path[i]))
                    {
                        errs() << "   CallInst: ";
                        call_inst->dump();

                        // Identify the argument argument of the current function that is
                        // the shared memory object
                        auto *last_inst = path[i - 1];
                        Value *matching_argument = nullptr;

                        for (unsigned int j = 0; j < call_inst->arg_size(); j++)
                        {
                            if (last_inst == call_inst->getArgOperand(j))
                            {
                                Function *called_function = call_inst->getCalledFunction();
                                if (called_function->arg_begin() + j <
                                    called_function->arg_end())
                                {
                                    matching_argument = called_function->arg_begin() + j;
                                }
                            }
                        }

                        builder.SetInsertPoint(store);
                        Value *void_ptr =
                            builder.CreateBitCast(matching_argument, Type::getInt8PtrTy(Ctx));
                        args.insert(args.begin(), void_ptr);
                        break;
                    }
                }
            }
            else
            {
                builder.SetInsertPoint(store);
                Value *void_ptr = builder.CreateBitCast(path[0], Type::getInt8PtrTy(Ctx));
                args.insert(args.begin(), void_ptr);
            }

            // Do the acutal replacement of the load instruction with a call to the cato
            // runtime library
            if (args.size() >= 3)
            {
                // WIP
                // TODO only allocate a new value if the original store had a constant
                // value that was stored
                builder.SetInsertPoint(store->getFunction()->getEntryBlock().getFirstNonPHI());
                Value *store_value = builder.CreateAlloca(store->getValueOperand()->getType());
                builder.SetInsertPoint(store);
                builder.CreateStore(store->getOperand(0), store_value);
                Value *void_ptr = builder.CreateBitCast(store_value, Type::getInt8PtrTy(Ctx));
                args.insert(args.begin() + 1, void_ptr);
                Value *new_store_call =
                    builder.CreateCall(runtime.functions.shared_memory_store, args);
                store->replaceAllUsesWith(new_store_call);
                store->eraseFromParent();
            }
        }

        // Replace freeing of shared memory with corresponding call to the cato runtime
        // library
        for (auto &path : free_paths)
        {
            if (auto *free_call = dyn_cast<CallInst>(path.back()))
            {
                Debug(errs() << "Analyzing free call instruction:\n";);
                Debug(free_call->dump(););

                for (int i = path.size() - 2; i >= 0; i--)
                {
                    Debug(errs() << " |";);
                    Debug(path[i]->dump(););
                }

                builder.SetInsertPoint(free_call);
                auto new_free_call = builder.CreateCall(runtime.functions.shared_memory_free,
                                                        free_call->getOperand(0));
                free_call->replaceAllUsesWith(new_free_call);
                free_call->eraseFromParent();
            }
            else
            {
                errs() << "Error: free path ends in non call instruction\n";
            }
        }
    }
}

/**
 * Searches the IR code for all __kmpc_fork_call calls and creates
 * a Microtask object for each one.
 **/
std::vector<std::unique_ptr<Microtask>> CatoPass::find_microtasks(Module &M)
{
    std::vector<std::unique_ptr<Microtask>> microtasks;
    auto kmpc_fork_call_users = get_function_users(M, "__kmpc_fork_call");
    for (auto &fork_call : kmpc_fork_call_users)
    {
        if (auto *fork_call_inst = dyn_cast<CallInst>(fork_call))
        {
            microtasks.push_back(std::make_unique<Microtask>(fork_call_inst));
        }
    }

    return microtasks;
}

/**
 * Replaces the __kmpc_fork_call instruction for each Microtask with a direct call to the
 * microtask function. This effectively eliminates the OpenMP functionality of the code
 * and makes each MPI process call the microtask function once.
 **/
void CatoPass::replace_fork_calls(Module &M, RuntimeHandler &runtime,
                                  std::vector<std::unique_ptr<Microtask>> &microtasks)
{
    LLVMContext &Ctx = M.getContext();
    IRBuilder<> builder(Ctx);

    for (auto &microtask : microtasks)
    {
        auto *fork_call_inst = microtask->get_fork_call();

        if (auto *num_shared_vars = dyn_cast<ConstantInt>(fork_call_inst->getArgOperand(1)))
        {
            // Construct arguments for the microtask function call
            std::vector<Value *> args;

            // The first to arguments are only used by OpenMP and can therefore be passed a
            // nullptr
            args.push_back(Constant::getNullValue(Type::getInt32PtrTy(Ctx)));
            args.push_back(Constant::getNullValue(Type::getInt32PtrTy(Ctx)));

            // Copy the passed arguments from the __kmpc_fork_call instruction
            for (int i = 0; i < num_shared_vars->getSExtValue(); i++)
            {
                args.push_back(fork_call_inst->getArgOperand(3 + i));
            }

            // Replace the fork_call with a direct call to the microtask function
            builder.SetInsertPoint(fork_call_inst);
            builder.CreateCall(microtask->get_function(), args);
            builder.CreateCall(runtime.functions.mpi_barrier);
            fork_call_inst->eraseFromParent();
        }
    }
}

/**
 * Replaces all memory allocations with calls to 'allocate_shared_memory'
 * from the cato runtime library.
 **/
void CatoPass::replace_memory_allocations(Module &M, RuntimeHandler &runtime)
{
    LLVMContext &Ctx = M.getContext();
    IRBuilder<> builder(Ctx);

    auto alloc_calls = find_memory_allocations(M);

    for (auto &allocation : alloc_calls)
    {
        CallInst *inst = allocation->get_allocation_call();
        Value *size = allocation->get_allocation_size();
        Type *type = allocation->get_allocation_type();

        builder.SetInsertPoint(inst);

        std::vector<Value *> args = {size, builder.getInt32(get_mpi_datatype(type)),
                                     builder.getInt32(get_pointer_depth(type))};

        auto new_call = builder.CreateCall(runtime.functions.allocate_shared_memory, args);
        new_call->takeName(inst);
        inst->replaceAllUsesWith(new_call);
        inst->eraseFromParent();
    }
}

/**
 * This function goes through all Microtasks and modifies the existing parallel for loops
 *for the use with MPI. To achieve this the lower- and upper-bound values of the for loops
 *are modified for each MPI process.
 **/
void CatoPass::replace_parallel_for(Module &M, RuntimeHandler &runtime,
                                    std::vector<std::unique_ptr<Microtask>> &microtasks)
{
    for (auto &microtask : microtasks)
    {
        std::vector<ParallelForData> *parallel_for_data_vec = microtask->get_parallel_for();
        // The Microtask does not contain a parallel for loop.
        // if (parallel_for_data == nullptr)
        // {
        //     continue;
        // }
        if (parallel_for_data_vec != nullptr)
        {
            for (ParallelForData parallel_for_data : *parallel_for_data_vec)
            {
                Debug(errs() << "Replacing parallel for in Microtask.\n";);
                Debug(errs() << "    ";);
                Debug(parallel_for_data.init->dump(););
                Debug(errs() << "    ";);
                Debug(parallel_for_data.fini->dump(););

                // Get the necessary information about the loop
                Value *lower_bound = parallel_for_data.init->getOperand(4);
                Value *upper_bound = parallel_for_data.init->getOperand(5);
                Value *increment = parallel_for_data.init->getOperand(7);
                Debug(errs() << "Loop lower bound: ");
                Debug(lower_bound->dump(););
                Debug(errs() << "Loop upper bound: ";);
                Debug(upper_bound->dump(););
                Debug(errs() << "Loop increment: ";);
                Debug(increment->dump(););

                // Replace the calls to the OpenMP Runtime Library by calls to the CATO
                // Runtime Library
                LLVMContext &Ctx = M.getContext();
                IRBuilder<> builder(parallel_for_data.init);

                std::vector<Value *> args = {lower_bound, upper_bound, increment};

                // Modify the lower and upper bound values
                CallInst *new_call = nullptr;
                if (lower_bound->getType() == Type::getInt32PtrTy(Ctx))
                {
                    new_call = builder.CreateCall(
                        runtime.functions.modify_parallel_for_bounds_4, args);
                }
                else if (lower_bound->getType() == Type::getInt64PtrTy(Ctx))
                {
                    new_call = builder.CreateCall(
                        runtime.functions.modify_parallel_for_bounds_8, args);
                }
                // Replace and remove OpenMP Runtime Library calls
                parallel_for_data.init->replaceAllUsesWith(new_call);
                parallel_for_data.init->eraseFromParent();
                parallel_for_data.fini->eraseFromParent();
            }
        }
    }
}

/**
 * This function replaces OpenMP reduction operations in the given Microtasks
 * into CATO reduction operations.
 **/
void CatoPass::replace_reductions(Module &M, RuntimeHandler &runtime,
                                  std::vector<std::unique_ptr<Microtask>> &microtasks)
{
    for (auto &microtask : microtasks)
    {
        std::vector<ReductionData> *reduction_data_vec = microtask->get_reductions();
        if (reduction_data_vec != nullptr)
        {
            for (ReductionData reduction_data : *reduction_data_vec)
            {
                Debug(errs() << "Microtask contains a reduction operation.\n";);
                Debug(errs() << "    Reduce: ";);
                Debug(reduction_data.reduce->dump(););
                Debug(errs() << "    End Reduce: ";);
                Debug(reduction_data.end_reduce->dump(););

                Value *num_reduction_variables = reduction_data.reduce->getOperand(2);
                Value *reduction_data_size = reduction_data.reduce->getOperand(3);
                Function *reduction_function = dyn_cast<Function>(
                    reduction_data.reduce->getOperand(5)->stripPointerCasts());
                // To get to the declaration of the reduction_lst we need to extract
                // it from the bitcast to void which is given to the kmpc call
                Value *reduction_lst = nullptr;
                if (auto *bitcast =
                        dyn_cast<BitCastInst>(reduction_data.reduce->getOperand(4)))
                {
                    reduction_lst = bitcast->getOperand(0);
                }
                // To get the local variable to be reduced we look for stores to the
                // reduction_lst. A pointer to the local variable will be stored into the
                // reduction_lst at some point.
                Value *local_reduction_var = nullptr;
                for (User *user : reduction_lst->users())
                {
                    if (auto *gep = dyn_cast<GetElementPtrInst>(user))
                    {
                        for (User *user2 : gep->users())
                        {
                            if (auto *store = dyn_cast<StoreInst>(user2))
                            {
                                local_reduction_var = store->getValueOperand();
                            }
                        }
                    }
                }

                Debug(errs() << "    Number reduction variables: ";);
                Debug(num_reduction_variables->dump(););
                Debug(errs() << "    Reduction data size: ";);
                Debug(reduction_data_size->dump(););
                Debug(errs() << "    Reduction reduction_lst: ";);
                Debug(reduction_lst->dump(););
                Debug(errs() << "    Reduction local var: ";);
                Debug(local_reduction_var->dump(););

                // The __kmpc_reduce... function calls are always followed by a switch
                // statement which handles their return values and implements the actual
                // reduction. The two cases of the switch statement will be removed and the
                // control flow will be redirected to the default case, where the CATO
                // version of the reduction operation is inserted.
                if (auto *switch_inst =
                        dyn_cast<SwitchInst>(reduction_data.reduce->getNextNode()))
                {
                    BasicBlock *case_default = switch_inst->getDefaultDest();
                    BasicBlock *case1 = switch_inst->getSuccessor(1);
                    BasicBlock *case2 = switch_inst->getSuccessor(2);
                    AtomicRMWInst::BinOp bin_op = AtomicRMWInst::BinOp::BAD_BINOP;
                    Value *reduction_target = nullptr;
                    // Reduction operations on integer and floating point create different
                    // IR
                    bool integer_type = true;

                    // To determine the type of the reduction operation we search for the
                    // case2 branch of the switch statement for an AtomicRMWInst. This
                    // instruction is used by the OpenMP code to reduce the result of the
                    // local variable reduction with its preexisting value
                    Debug(errs() << "";);
                    for (Instruction &instr : *case2)
                    {
                        if (auto *atomicrmw = dyn_cast<AtomicRMWInst>(&instr))
                        {
                            bin_op = atomicrmw->getOperation();
                            switch (bin_op)
                            {
                            case AtomicRMWInst::BinOp::Add:
                                Debug(errs() << "    Reduction operation: ADD\n";);
                                break;
                            case AtomicRMWInst::BinOp::Max:
                                Debug(errs() << "    Reduction operation: MAX\n";);
                                break;
                            case AtomicRMWInst::BinOp::Min:
                                Debug(errs() << "    Reduction operation: MIN\n";);
                                break;
                            default:
                                errs() << "    Reduction operation: UNKWOWN\n";
                                break;
                            }

                            // We can get a pointer to the shared variable of the reduction
                            // from the AtomicRMWInst that reduces it with the result of
                            // the local reduction
                            reduction_target = atomicrmw->getOperand(0);
                            Debug(errs() << "    Shared variable reduction target: ";);
                            Debug(reduction_target->dump());
                        }
                        // If the reduction is not on an integer type the IR code differs
                        // and an additional branch is created from which we have to
                        // extract the needed information about the reduction operation
                        else if (auto *branch = dyn_cast<BranchInst>(&instr))
                        {
                            if (bin_op == AtomicRMWInst::BinOp::BAD_BINOP)
                            {
                                integer_type = false;
                                Debug(errs() << "   Reduction on non integer type\n";);

                                for (auto &instr : *(branch->getSuccessor(0)))
                                {
                                    // ADD/SUB reductions both produce the same IR
                                    if (auto *binary_op = dyn_cast<BinaryOperator>(&instr))
                                    {
                                        if (binary_op->getOpcode() ==
                                            Instruction::BinaryOps::FAdd)
                                        {
                                            Debug(errs() << "    Reduction operation: ADD\n";);
                                            bin_op = AtomicRMWInst::BinOp::Add;
                                            break;
                                        }
                                    }
                                    // For MAX/MIN we need to find the compare instruction
                                    // and extract the type of comparison
                                    else if (auto *cmp_inst = dyn_cast<FCmpInst>(&instr))
                                    {
                                        if (cmp_inst->getPredicate() ==
                                            CmpInst::Predicate::FCMP_OGT)
                                        {
                                            Debug(errs() << "    Reduction operation: MAX\n";);
                                            bin_op = AtomicRMWInst::BinOp::Max;
                                            break;
                                        }
                                        else if (cmp_inst->getPredicate() ==
                                                 CmpInst::Predicate::FCMP_OLT)
                                        {
                                            Debug(errs() << "    Reduction operation: MIN\n";);
                                            bin_op = AtomicRMWInst::BinOp::Min;
                                            break;
                                        }
                                        else
                                        {
                                            errs() << "Reduction operation: UNKNOWN\n";
                                        }
                                    }
                                }

                                // Find the pointer to the shared variable that gets the
                                // reduction result
                                for (Instruction &instr : *case2)
                                {
                                    if (auto *load = dyn_cast<LoadInst>(&instr))
                                    {
                                        if (auto *bitcast =
                                                dyn_cast<BitCastInst>(load->getOperand(0)))
                                        {
                                            reduction_target = bitcast->getOperand(0);
                                            Debug(errs() << "    Shared variable reduction "
                                                            "target: ";);
                                            Debug(reduction_target->dump());
                                        }
                                    }
                                }
                            }
                        }
                    }

                    LLVMContext &Ctx = M.getContext();
                    IRBuilder<> builder(switch_inst);

                    // Replace the switch statement with a direct branch to the default
                    // case, where we will insert the CATO reduction code
                    builder.CreateBr(case_default);
                    switch_inst->eraseFromParent();
                    reduction_data.reduce->eraseFromParent();
                    reduction_data.end_reduce->eraseFromParent();

                    // Do a MPI reduction for the local values of the reduction variable
                    // after this all processes will have the reduced value in their local
                    // variable
                    std::vector<Value *> args = {
                        local_reduction_var, builder.getInt32(bin_op),
                        builder.getInt32(get_mpi_datatype(reduction_target))};
                    builder.SetInsertPoint(case_default->getFirstNonPHI());
                    builder.CreateCall(runtime.functions.reduce_local_vars, args);

                    // Now local_reduction var contains the reduction result for all local
                    // variables and it needs to be reduced once more with the initial
                    // value of the reduction target variable This only needs to be done by
                    // one MPI process
                    // TODO clean up this part of the code
                    CallInst *mpi_rank = builder.CreateCall(runtime.functions.get_mpi_rank);
                    BasicBlock *split_block =
                        SplitBlock(case_default, &*builder.GetInsertPoint());
                    BasicBlock *master_reduction_block =
                        BasicBlock::Create(Ctx, "master_reduction", case_default->getParent());
                    builder.SetInsertPoint(case_default->getTerminator());
                    Value *condition = builder.CreateICmpEQ(mpi_rank, builder.getInt32(0));
                    builder.CreateCondBr(condition, master_reduction_block, split_block);
                    case_default->getTerminator()->eraseFromParent();

                    builder.SetInsertPoint(master_reduction_block);

                    Value *load = builder.CreateLoad(
                        dyn_cast<BitCastInst>(local_reduction_var)
                            ->getOperand(0)
                            ->getType()
                            ->getPointerElementType(),
                        dyn_cast<BitCastInst>(local_reduction_var)->getOperand(0));

                    if (integer_type)
                    {
                        auto *add =
                            builder.CreateAtomicRMW(bin_op, reduction_target, load,
                                                    MaybeAlign(), AtomicOrdering::Monotonic);
                    }
                    // For floating point types we need to add more specific IR code
                    else
                    {
                        if (bin_op == AtomicRMWInst::BinOp::Add)
                        {
                            auto *add = builder.CreateAtomicRMW(
                                AtomicRMWInst::BinOp::FAdd, reduction_target, load,
                                MaybeAlign(), AtomicOrdering::Monotonic);
                        }
                        // TODO see if shared_value load/store calls are necessary here
                        else if (bin_op == AtomicRMWInst::BinOp::Max)
                        {
                            auto *target_load = builder.CreateLoad(
                                reduction_target->getType()->getPointerElementType(),
                                reduction_target);
                            auto *fcmp = builder.CreateFCmp(CmpInst::Predicate::FCMP_OGT,
                                                            target_load, load);
                            auto *select = builder.CreateSelect(fcmp, target_load, load);
                            auto *store = builder.CreateStore(select, reduction_target);
                        }
                        // TODO see if shared_value load/store calls are necessary here
                        else if (bin_op == AtomicRMWInst::BinOp::Min)
                        {
                            auto *target_load = builder.CreateLoad(
                                reduction_target->getType()->getPointerElementType(),
                                reduction_target);
                            auto *fcmp = builder.CreateFCmp(CmpInst::Predicate::FCMP_OLT,
                                                            target_load, load);
                            auto *select = builder.CreateSelect(fcmp, target_load, load);
                            auto *store = builder.CreateStore(select, reduction_target);
                        }
                        else
                        {
                            // TODO
                        }
                    }
                    builder.CreateBr(split_block);

                    // TODO erase unneeded basicblocks case1/case2 and possible successors
                    // This might also be done automatically by the following llvm passes
                    // need to check final IR output to confirm
                    reduction_function->eraseFromParent();
                }
                else
                {
                    errs() << "Error: Unknown IR pattern for a reduction pragma!\n";
                }
            }
        }
    }
}

/**
 * This function replaces all OpenMP critical sections inside the given Microtasks with a
 * CATO critical section
 **/
void CatoPass::replace_criticals(Module &M, RuntimeHandler &runtime,
                                 std::vector<std::unique_ptr<Microtask>> &microtasks)
{
    for (auto &microtask : microtasks)
    {
        std::vector<CriticalData> *critical_data_vec = microtask->get_critical();
        if (critical_data_vec != nullptr)
        {
            IRBuilder<> builder(M.getContext());
            LLVMContext &Ctx = M.getContext();

            // Create a MPI Mutex for the microtask
            builder.SetInsertPoint(
                microtask->get_function()->getEntryBlock().getFirstNonPHI());
            Value *mpi_mutex_call =
                builder.CreateCall(runtime.functions.critical_section_init);

            // Replace the begin an end of the OpenMP critical section with mutex
            // acquisition / release
            for (CriticalData critical_data : *critical_data_vec)
            {
                Debug(errs() << "Microtask contains a critical section.\n";);
                Debug(errs() << "    Critical: ";);
                Debug(critical_data.critical->dump(););
                Debug(errs() << "    End Critical: ";);
                Debug(critical_data.end_critical->dump(););

                builder.SetInsertPoint(critical_data.critical);
                Value *critical_enter_call = builder.CreateCall(
                    runtime.functions.critical_section_enter, mpi_mutex_call);
                critical_data.critical->replaceAllUsesWith(critical_enter_call);
                critical_data.critical->eraseFromParent();

                builder.SetInsertPoint(critical_data.end_critical);
                Value *critical_leave_call = builder.CreateCall(
                    runtime.functions.critical_section_leave, mpi_mutex_call);
                critical_data.end_critical->replaceAllUsesWith(critical_leave_call);
                critical_data.end_critical->eraseFromParent();
            }

            // Destroy the mutex
            auto return_instructions =
                get_instruction_in_function<ReturnInst>(microtask->get_function());
            for (auto &ret : return_instructions)
            {
                builder.SetInsertPoint(ret);
                builder.CreateCall(runtime.functions.critical_section_finalize,
                                   mpi_mutex_call);
            }
        }
    }
}

/**
 * Replaces all calls that free allocated memory with a free memory call to the CATO
 *Runtime Library This is needed because some shared memory objects need more than a free
 *call to their base pointer for all memory and MPI objects to be correctly deallocated
 **/
void CatoPass::replace_memory_deallocations(Module &M, RuntimeHandler &runtime)
{
    LLVMContext &Ctx = M.getContext();
    IRBuilder<> builder(Ctx);

    std::vector<CallInst *> dealloc_calls = find_memory_deallocations(M);

    for (CallInst *&deallocation : dealloc_calls)
    {
        builder.SetInsertPoint(deallocation);

        Value *new_dealloc_call = builder.CreateCall(runtime.functions.shared_memory_free,
                                                     deallocation->getOperand(0));

        deallocation->replaceAllUsesWith(new_dealloc_call);
        deallocation->eraseFromParent();
    }
}

/**
 * Only use during development!
 * Inserting the test_func function into the program
 * Used for easier testing of rtlib functions.
 * just change the content of the test_func in rtlib an call this function to
 * automatically insert it at the beginning of the program
 **/
void CatoPass::insert_test_func(Module &M, RuntimeHandler &runtime)
{
    IRBuilder<> builder(M.getContext());
    LLVMContext &Ctx = M.getContext();
    auto *entry_block = runtime.get_entry_block();

    builder.SetInsertPoint(entry_block->getTerminator());

    // Construct the argument list
    std::vector<Value *> args;
    args.push_back(builder.getInt32(3));
    args.push_back(builder.getInt64(0));
    args.push_back(builder.getInt64(100));
    args.push_back(builder.getInt64(42));

    builder.CreateCall(runtime.functions.test_func, args);
}

/**
 * Entry point of the pass
 **/
bool CatoPass::runOnModule(Module &M)
{

    Debug(errs() << "*----------------------------------*\n";);
    Debug(errs() << "|      IR CODE BEFORE THE PASS:    |\n";);
    Debug(errs() << "*----------------------------------*\n";);
    Debug(M.dump(););

    Debug(errs() << "*----------------------------------*\n";);
    Debug(errs() << "|            DEBUG OUTPUT:         |\n";);
    Debug(errs() << "*----------------------------------*\n";);

    RuntimeHandler runtime(M);

    if (cato_logging)
    {
        runtime.insert_cato_init_and_fin(nullptr, true);
    }
    else
    {
        runtime.insert_cato_init_and_fin();
    }

    runtime.replace_omp_functions();

    std::vector<std::unique_ptr<Microtask>> microtasks = find_microtasks(M);

    replace_fork_calls(M, runtime, microtasks);

    replace_memory_allocations(M, runtime);

    replace_sequential_shared_memory_accesses(M, runtime);

    replace_microtask_shared_memory_accesses(M, runtime, microtasks);

    replace_parallel_for(M, runtime, microtasks);

    replace_reductions(M, runtime, microtasks);

    replace_criticals(M, runtime, microtasks);

    replace_memory_deallocations(M, runtime);

    // insert_test_func(M, runtime);
    Debug(errs() << "*----------------------------------*\n";);
    Debug(errs() << "|      IR CODE AFTER THE PASS:     |\n";);
    Debug(errs() << "*----------------------------------*\n";);
    Debug(M.dump(););

    return true;
}

/* ------------------------------ Register Pass ----------------------------- */
PassPluginLibraryInfo getCatoPluginInfo()
{
    return {LLVM_PLUGIN_API_VERSION, "Cato", LLVM_VERSION_STRING, [](PassBuilder &PB) {
                PB.registerPipelineParsingCallback([](StringRef Name, ModulePassManager &MPM,
                                                      ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "Cato")
                    {
                        MPM.addPass(CatoPass());
                        return true;
                    }
                    return false;
                });
            }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::PassPluginLibraryInfo llvmGetPassPluginInfo()
{
    return getCatoPluginInfo();
}