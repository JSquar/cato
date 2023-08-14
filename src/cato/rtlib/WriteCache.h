/*
 * File: WriteCache.h
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Mon Aug 14 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#ifndef CATO_RTLIB_WRITECACHE_H
#define CATO_RTLIB_WRITECACHE_H

#include <unordered_map>
#include <mpi.h>
#include "WriteCacheLine.h"

/**
 * Instead of writing single elements to remote processes, the elements are aggregated in this cache.
 * Upon dropping the cache, the contents are written to the target windows and ranks in large buffers
 * (currently one buffer per rank for each window). The contents per rank can also be written to the target
 * before the forced flush. This is controlled via the env variable CATO_FLUSH_WRITE_AFTER. Setting this leads
 * to the cache contents being written to the target rank once that many elements have been aggregated.
 * The cache itself is a key-value store. Each value, being a WriteCacheLine object, represents a MemoryAbstraction.
 * Hence the key is the address of the MemoryAbstraction.
 **/
class WriteCache
{
  private:
    std::unordered_map<void*, WriteCacheLine> _cache;

    bool _enabled;

    size_t _flush_rank_after_num_elements;

  public:
    WriteCache(bool enabled, int flush_rank_after)
        : _enabled{enabled}, _flush_rank_after_num_elements{static_cast<size_t>(flush_rank_after)}{};

    /**
     * Stores the content of data in the cache. If the target of this operation is a previously unseen
     * MemoryAbstraction, a new key-value pair is created first and then the content of data is stored.
     **/
    void store_in_cache(void* data, int target_rank, long displacement, void* mem_abstraction_base, MPI_Datatype type, MPI_Win win);

    void clear_cache();

    bool cache_enabled() const {return _enabled;}
};


#endif