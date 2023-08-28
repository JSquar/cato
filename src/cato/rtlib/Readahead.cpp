/*
 * File: Readahead.cpp
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Mon Aug 28 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#include "Readahead.h"
#include <mpi.h>

void* performReadahead(MemoryAbstractionDefault* mem_abstraction, void* base_ptr, CacheHandler* cache_handler,
                        const std::vector<long>& initial_indices, std::pair<int,long> rank_and_disp, long count)
{
    void* buf = malloc(mem_abstraction->_type_size * count);
    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, rank_and_disp.first, 0, mem_abstraction->_mpi_window);
    MPI_Get(buf, count, mem_abstraction->_type, rank_and_disp.first, rank_and_disp.second, count, mem_abstraction->_type,
            mem_abstraction->_mpi_window);
    MPI_Win_unlock(rank_and_disp.first, mem_abstraction->_mpi_window);

    for (long i = 0; i < count; i++)
    {
        //The resulting cache elem index might not actually exist, but there is a
        //certain overhead attached to keeping track of "legal" indices. If the index does
        //not actually exist, the element will just stay in the cache untouched until dropped.
        //TODO make some measurements with and without index mngmt
        std::vector<long> cache_elem_index = initial_indices;
        cache_elem_index.back() += i;
        void* addr = static_cast<char*>(buf) + i*mem_abstraction->_type_size;
        auto key = cache_handler->make_cache_key(base_ptr, cache_elem_index);
        CacheElement value {addr, static_cast<size_t>(mem_abstraction->_type_size)};
        cache_handler->get_read_cache().store_in_cache(key, value);
    }
    return buf;
}