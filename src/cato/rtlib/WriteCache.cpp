/*
 * File: WriteCache.cpp
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Sun Sep 03 2023
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

void WriteCache::store_in_cache(void* mem_abstraction_base, MPI_Datatype type, MPI_Win win, void* data, int target_rank, long displacement)
{
    if (_enabled)
    {
        auto map_entry = _cache.insert({mem_abstraction_base, {win, type}});
        WriteCacheLine& cache_line = map_entry.first->second;
        cache_line.insert_element(data, target_rank, displacement, _flush_rank_after_num_elements);
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