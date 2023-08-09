/*
 * File: Cache.h
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Wed Aug 09 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#ifndef CATO_RTLIB_CACHE_H
#define CATO_RTLIB_CACHE_H

#include <unordered_map>
#include <cstring>
#include <memory>
#include <vector>

#include "Cacheline.h"
#include "Indexline.h"
#include "Writecache.h"

class Cache
{
  private:

    struct hash_combiner
    {
        size_t operator()(std::pair<void*,std::vector<long>> const& key) const
        {
            size_t seed = (size_t) key.first;
            for (auto elem : key.second)
            {
                //https://stackoverflow.com/a/27216842
                //seed ^= elem + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                //https://stackoverflow.com/a/72073933
                elem = ((elem >> 16) ^ elem) * 0x45d9f3b;
                elem = ((elem >> 16) ^ elem) * 0x45d9f3b;
                elem = (elem >> 16) ^ elem;
                seed ^= elem + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };

    std::unordered_map<std::pair<void*,std::vector<long>>, Cacheline, hash_combiner> _cache;

    std::unordered_map<std::pair<void*,std::vector<long>>, Indexline, hash_combiner> _index_cache;

    std::unordered_map<void*, Writecache> _write_cache;

    bool _cache_enabled;

    bool _index_cache_enabled;

    bool _write_cache_enabled;

    int _read_ahead;

  public:
    Cache();

    void store_in_cache(void* src, size_t size, void* base_ptr, const std::vector<long>& initial_indices);

    void print_cache();

    Cacheline* find_cacheline(void* const base_ptr, const std::vector<long>& indices);

    void store_in_index_cache_local(void* target, size_t size, void* base_ptr, const std::vector<long>& initial_indices);

    void store_in_index_cache_remote(MemoryAbstraction* mem_abstraction, long index, void* base_ptr, const std::vector<long>& initial_indices);

    Indexline* find_index(void* const base_ptr, const std::vector<long>& indices);

    void drop_cache();

    long get_read_ahead();

    void store_in_write_cache(void* data, int target_rank, long displacement, void* mem_abstraction_base, MPI_Datatype type, MPI_Win win);

    void clear_write_cache();

    bool write_cache_enabled() const {return _write_cache_enabled;}
};

#endif