/*
 * File: CacheHandler.h
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Mon Aug 14 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#ifndef CATO_RTLIB_CACHEHANDLER_H
#define CATO_RTLIB_CACHEHANDLER_H

#include <unordered_map>
#include <cstring>
#include <memory>
#include <vector>

#include "CacheElement.h"
#include "IndexCacheElement.h"
#include "WriteCacheLine.h"
#include "ReadCache.h"
#include "WriteCache.h"

/**
 * This class manages and provides access to the different caches. There are three caches in total,
 * all of them are key-value stores.
 * The first one is the index cache. This is a TLB-like cache that is supposed to minimize the index
 * calculations that are necessary to find the correct memory abstractions. The key is made up of the
 * original address and the indices for the access. The elements are IndexCacheElements (see class for more info).
 * The second one is the read cache. This cache is used to store elements from remote loads, so that subsequent
 * accesses to it do not require another MPI call, but rather a local access. It uses the same key as the index
 * cache (typedef'd to CatoCacheKey), the elements are CacheElement objects (see class for more info). Note that
 * both of these caches have singular values as their elements.
 * The third cache is a write cache. This cache aggregates write accesses. See the class for more info.
 * The caches can be enabled separately, see CacheHandler.cpp for the respective env variables. There is also a
 * readahead option for loads to get more than just one value in each load operation from remote processes,
 * alongside an option for the write cache to write elements to the target earlier (see WriteCache.h).
 **/

class CacheHandler
{
  public:
    typedef std::pair<void*,std::vector<long>> CatoCacheKey;

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

    ReadCache<CatoCacheKey, CacheElement, hash_combiner> _read_cache;

    ReadCache<CatoCacheKey, IndexCacheElement, hash_combiner> _index_cache;

    WriteCache _write_cache;

    int _read_ahead;

    bool is_enabled(const char* env_var);

    int env_var_value(const char* env_var);

  public:

    CacheHandler();

    /**
     * The info used for the key is the base pointer in the original access
     * alongside the indices. This function transforms them into the key.
     **/
    CatoCacheKey make_cache_key(void* const base_ptr, const std::vector<long>& indices);

    void clear_write_cache();

    void clear_read_cache();

    ReadCache<CatoCacheKey, CacheElement, hash_combiner>& get_read_cache() {return _read_cache;}

    ReadCache<CatoCacheKey, IndexCacheElement, hash_combiner>& get_index_cache() {return _index_cache;}

    WriteCache& get_write_cache() {return _write_cache;}

    long get_read_ahead() const {return _read_ahead;}
};

#endif