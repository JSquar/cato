/*
 * File: Readahead.cpp
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Sun Sep 03 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#include "Readahead.h"
#include <mpi.h>
#include <cstdlib>

#include "PerformanceMetrics.h"

namespace
{
    MPI_Datatype create_readahead_datatype(int stride, MPI_Datatype old_dt)
    {
        MPI_Datatype dt;
        int type_size;
        MPI_Type_size(old_dt, &type_size);
        MPI_Type_create_resized(old_dt, 0, stride*type_size, &dt);
        MPI_Type_commit(&dt);
        return dt;
    }
}


void* performReadahead(MemoryAbstractionDefault* mem_abstraction, void* base_ptr, CacheHandler* const cache_handler,
                        const std::vector<long>& initial_indices, std::pair<int,long> rank_and_disp, std::pair<int,long> readahead_count_stride)
{
    int count = readahead_count_stride.first;
    long stride = readahead_count_stride.second;
    void* buf = std::malloc(mem_abstraction->_type_size * count);

    if (mem_abstraction->_readahead_dt == MPI_DATATYPE_NULL)
    {
        mem_abstraction->_readahead_dt = create_readahead_datatype(stride, mem_abstraction->_type);
    }

    MPI_Win_lock(MPI_LOCK_SHARED, rank_and_disp.first, 0, mem_abstraction->_mpi_window);
    MPI_Get(buf, count, mem_abstraction->_type, rank_and_disp.first,
            rank_and_disp.second, count, mem_abstraction->_readahead_dt, mem_abstraction->_mpi_window);
    MPI_Win_unlock(rank_and_disp.first, mem_abstraction->_mpi_window);

    for (long i = 0; i < count; i++)
    {
        //The resulting cache elem index might not actually exist, but there is a
        //certain overhead attached to keeping track of "legal" indices. If the index does
        //not actually exist, the element will just stay in the cache untouched until dropped.
        //TODO make some measurements with and without index mngmt
        std::vector<long> cache_elem_index = initial_indices;
        cache_elem_index.back() += i*stride;
        void* addr = static_cast<char*>(buf) + i*mem_abstraction->_type_size;
        cache_handler->store_in_read_cache(base_ptr, cache_elem_index, addr, static_cast<size_t>(mem_abstraction->_type_size));
    }

    report_elements_read_ahead(count);

    return buf;
}