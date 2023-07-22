/*
 * File: Cache.cpp
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Sat Jul 22 2023
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
    _cache_enabled = (cache_enable != NULL && std::strcmp(cache_enable, "1") == 0);
    Debug(std::cout << "Cache enabled: " << _cache_enabled << "\n";);
}

void Cache::store_in_cache(void* src, size_t size, void* base_ptr, const std::vector<long> initial_indices)
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
        std::cout << _cache.hash_function()(cache_line.first) << ": " << *(int*)cache_line.second.getData() << "\n";
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

void Cache::drop_cache()
{
    _cache.clear();
}
