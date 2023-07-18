/*
 * File: Cache.h
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Tue Jul 18 2023
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

class Cache
{
  private:

    struct deleter
    {
        void operator()(void* d) const noexcept
        {
            std::free(d);
        }
    };

    struct hash_combiner
    {
        size_t operator()(std::pair<void*,std::vector<long>> const&) const
        {
            return 0;
        }
    };

    std::unordered_map<std::pair<void*,std::vector<long>>, std::unique_ptr<void, deleter>, hash_combiner> _cache;

  public:
    void print_something();

    void store_in_cache(void* src, size_t size, void* base_ptr, std::vector<long> initial_indices);

    void print_cache();

};

#endif