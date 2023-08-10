/*
 * File: WriteCache.cpp
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Thu Aug 10 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#include "WriteCache.h"

void WriteCache::store_in_cache(void* data, int target_rank, long displacement, void* mem_abstraction_base, MPI_Datatype type, MPI_Win win)
{
    if (_enabled)
    {
        auto map_entry = _cache.insert({mem_abstraction_base, {win, type}});
        WriteCacheLine& cache = map_entry.first->second;
        cache.insert_element(data, target_rank, displacement);
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