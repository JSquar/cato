/*
 * File: Cache.cpp
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Wed Aug 02 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#include <iostream>
#include <cstdlib>

#include "Cache.h"
#include "../debug.h"

Cache::Cache()
{
    char* cache_enable = std::getenv("CATO_ENABLE_CACHE");
    char* read_ahead = std::getenv("CATO_CACHE_READAHEAD");
    _cache_enabled = (cache_enable != NULL && std::strcmp(cache_enable, "1") == 0);

    if (!_cache_enabled || read_ahead == NULL)
    {
        _read_ahead = 0;
    }
    else
    {
        char* end;
        _read_ahead = std::strtol(read_ahead, &end, 0);
        if (*end)
        std::cerr << "Could not read value for CATO_CACHE_READAHEAD";
    }
    Debug(std::cout << "Cache enabled: " << _cache_enabled << "\n";);
    Debug(std::cout << "Cache readahead: " << _read_ahead << "\n";);
}

void Cache::store_in_cache(void* src, size_t size, void* base_ptr, const std::vector<long>& initial_indices)
{
    if (_cache_enabled)
    {
        Cacheline value {src, size};
        auto key = std::make_pair(base_ptr, initial_indices);
        _cache.insert_or_assign(std::move(key), std::move(value));
    }
}

void Cache::print_cache()
{
    std::cout << "START\n";
    for (auto const& cache_line: _cache)
    {
        std::cout << _cache.hash_function()(cache_line.first) << ": " << *(float*)cache_line.second.get_data() << "\n";
    }
    std::cout << "END\n";
}

Cacheline* Cache::find_cacheline(void* const base_ptr, const std::vector<long>& indices)
{
    if (_cache_enabled)
    {
        Cacheline* res = nullptr;
        auto entry = _cache.find({base_ptr, indices});
        if (entry != _cache.end())
        {
            res = &(entry->second);
        }
        return res;
    }
    return nullptr;
}

void Cache::store_in_index_cache_local(void* target, size_t size, void* base_ptr, const std::vector<long>& initial_indices)
{
    if (_cache_enabled)
    {
        Indexline value {target, size};
        auto key = std::make_pair(base_ptr, initial_indices);
        _index_cache.insert({key, value});
    }
}

void Cache::store_in_index_cache_remote(MemoryAbstraction* mem_abstraction, long index, void* base_ptr, const std::vector<long>& initial_indices)
{
    if (_cache_enabled)
    {
        Indexline value {mem_abstraction, index};
        auto key = std::make_pair(base_ptr, initial_indices);
        _index_cache.insert({key, value});
    }
}

Indexline* Cache::find_index(void* const base_ptr, const std::vector<long>& indices)
{
    if (_cache_enabled)
    {
        Indexline* res = nullptr;
        auto entry = _index_cache.find({base_ptr, indices});
        if (entry != _index_cache.end())
        {
            res = &(entry->second);
        }
        return res;
    }
    return nullptr;
}

void Cache::drop_cache()
{
    _cache.clear();
}

long Cache::get_read_ahead()
{
    return _read_ahead;
}
