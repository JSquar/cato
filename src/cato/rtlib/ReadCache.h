/*
 * File: ReadCache.h
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Sat Sep 30 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#ifndef CATO_RTLIB_READCACHE_H
#define CATO_RTLIB_READCACHE_H

#include <unordered_map>
#include <list>


template <class K, class V, class H = std::hash<K>>
class ReadCache
{
  private:
    std::unordered_map<K, V, H> _read_cache;

    std::list<typename std::unordered_map<K, V, H>::iterator> _lru_list;

    bool _enabled;

    size_t _capacity;

  public:
    ReadCache(bool enabled, size_t capacity) : _enabled{enabled}, _capacity{capacity}{};

    void store_in_cache(K& key, V& value)
    {
        if (_enabled)
        {
            auto entry = _read_cache.insert_or_assign(std::move(key), std::move(value));
            if (_capacity)
            {
                if (!entry.second)
                {
                    _lru_list.remove(entry.first);
                }
                _lru_list.push_front(entry.first);

                if (_lru_list.size() > _capacity)
                {
                    auto last_element = _lru_list.back();
                    _read_cache.erase(last_element);
                    _lru_list.pop_back();
                }
            }
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

                if (_capacity)
                {
                    _lru_list.remove(entry);
                    _lru_list.push_front(entry);
                }
            }
            return res;
        }
        return nullptr;
    }

    void clear_cache()
    {
        _read_cache.clear();
        _lru_list.clear();
    }

    bool cache_enabled() const {return _enabled;}
};

#endif