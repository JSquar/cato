/*
 * File: Readahead.cpp
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Thu Sep 07 2023
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

    int calculate_element_count_with_stride(std::pair<long,long> array_bounds, long displacement, int stride)
    {
        int num_elems_in_target = array_bounds.second - array_bounds.first + 1;
        int num_elems_left_after_displacement = num_elems_in_target - displacement;
        int num_elems_left_strided = num_elems_left_after_displacement / stride;

        if (num_elems_left_strided == 0)
        {
            return 1;
        }

        return num_elems_left_strided;
    }
}


void* perform_readahead(MemoryAbstractionDefault* mem_abstraction, void* base_ptr, CacheHandler* const cache_handler,
                        const std::vector<long>& initial_indices, const std::vector<long>& indices)
{
    auto rank_and_disp = mem_abstraction->get_target_rank_and_disp_for_offset(indices[0]);
    int stride = cache_handler->get_read_ahead_stride_for(base_ptr);
    Debug(std::cout << "Got stride " << stride << " for " << base_ptr << "\n";);

    std::pair<long,long> array_bounds = mem_abstraction->_array_ranges[rank_and_disp.first];
    int element_count_strided = calculate_element_count_with_stride(array_bounds, rank_and_disp.second, stride);
    int count = std::min(cache_handler->get_read_ahead(), element_count_strided);

    void* buf = std::malloc(mem_abstraction->_type_size * count);
    if (buf == nullptr)
    {
        throw std::bad_alloc();
    }

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