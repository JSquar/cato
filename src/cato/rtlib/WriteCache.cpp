/*
 * File: WriteCache.cpp
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Mon Sep 04 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#include "WriteCache.h"

WriteCache::WriteCache(bool enabled, int flush_rank_after) : _enabled{enabled}
{
    //Counts in MPI comm calls are ints
    if (flush_rank_after <= 0)
    {
        _flush_rank_after_num_elements = INT32_MAX;
    }
    else
    {
        _flush_rank_after_num_elements = static_cast<size_t>(flush_rank_after);
    }
}

void WriteCache::store_in_cache(void* data, MemoryAbstractionDefault* mem_abstraction, const std::vector<long> indices)
{
    if (_enabled)
    {
        auto rank_and_disp = mem_abstraction->get_target_rank_and_disp_for_offset(indices[0]);
        auto map_entry = _cache.insert({mem_abstraction->_base_ptr, {mem_abstraction->_mpi_window, mem_abstraction->_type}});
        WriteCacheLine& cache_line = map_entry.first->second;
        cache_line.insert_element(data, rank_and_disp.first, rank_and_disp.second, _flush_rank_after_num_elements);
    }
}

void WriteCache::clear_cache()
{
    if (_enabled)
    {
        for (auto& elem : _cache)
        {
            elem.second.clear_cache();
        }
    }
}