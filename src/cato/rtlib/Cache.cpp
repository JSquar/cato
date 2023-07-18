/*
 * File: Cache.cpp
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Tue Jul 18 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#include <iostream>
#include <cstdlib>

#include "Cache.h"


void Cache::print_something()
{
    std::cout << "CACHE STUFF\n";
}

void Cache::store_in_cache(void* src, size_t size, void* base_ptr, std::vector<long> initial_indices)
{
    auto value = std::unique_ptr<void, deleter>(std::malloc(size));
    std::memcpy(value.get(), src, size);
    auto key = std::make_pair(base_ptr, initial_indices);
    _cache.insert({key, std::move(value)});
}

void Cache::print_cache()
{
    std::cout << "START\n";
    for (auto const& cache_line: _cache)
    {
        std::cout << _cache.hash_function()(cache_line.first) << ": " << *(int*)cache_line.second.get() << "\n";
    }
    std::cout << "END\n";
}