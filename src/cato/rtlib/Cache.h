/*
 * File: Cache.h
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Thu Jul 27 2023
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

    bool _cache_enabled;

    int _read_ahead;

  public:
    Cache();

    void store_in_cache(void* src, size_t size, void* base_ptr, const std::vector<long>& initial_indices);

    void print_cache();

    Cacheline* find_cacheline(void* const base_ptr, const std::vector<long>& indices);

    void drop_cache();

    long get_read_ahead();
};

#endif