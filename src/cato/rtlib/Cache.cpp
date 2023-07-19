/*
 * File: Cache.cpp
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Wed Jul 19 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#include <iostream>
#include <cstdlib>

#include "Cache.h"

void Cache::store_in_cache(void* src, size_t size, void* base_ptr, std::vector<long> initial_indices)
{
    Cacheline value {src, size};
    auto key = std::make_pair(base_ptr, initial_indices);
    _cache.insert({std::move(key), std::move(value)});
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
    Cacheline* res = nullptr;
    auto entry = _cache.find({base_ptr, indices});
    if (entry != _cache.end())
    {
        res = &(entry->second);
    }
    return res;
}