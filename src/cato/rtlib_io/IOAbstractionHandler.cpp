#include "IOAbstractionHandler.h"
#include "../debug.h"

IOAbstractionHandler::IOAbstractionHandler(int ncid, int varid)
{
    _mpi_rank = rank;
    _mpi_size = size;
}

std::vector<std::unique_ptr<MemoryAllocation>> find_memory_allocations(Module &M)
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