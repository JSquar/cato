/*
 * File: ReadCache.h
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Thu Aug 10 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#ifndef CATO_RTLIB_READCACHE_H
#define CATO_RTLIB_READCACHE_H

#include <unordered_map>


template <class K, class V, class H = std::hash<K>>
class ReadCache
{
  private:
    std::unordered_map<K, V, H> _read_cache;

    bool _enabled;

  public:
    ReadCache(bool enabled) : _enabled{enabled}{};

    void store_in_cache(K& key, V& value)
    {
        if (_enabled)
        {
            _read_cache.insert_or_assign(std::move(key), std::move(value));
        }
    }

    V* find_element(K& key)
    {
        if (_enabled)
        {
            V* res = nullptr;
            auto entry = _read_cache.find(key);
            if (entry != _read_cache.end())
            {
                res = &(entry->second);
            }
            return res;
        }
        return nullptr;
    }

    void clear()
    {
        _read_cache.clear();
    }

    bool cache_enabled() const {return _enabled;}
};

#endif