/*
 * File: CacheHandler.cpp
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Thu Aug 10 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#include <iostream>
#include <cstdlib>

#include "CacheHandler.h"
#include "../debug.h"

CacheHandler::CacheHandler()
{
    char* read_cache_enable = std::getenv("CATO_ENABLE_READ_CACHE");
    char* write_cache_enable = std::getenv("CATO_ENABLE_WRITE_CACHE");
    char* index_cache_enable = std::getenv("CATO_ENABLE_INDEX_CACHE");
    char* read_ahead = std::getenv("CATO_CACHE_READAHEAD");

    _read_cache_enabled = (read_cache_enable != NULL && std::strcmp(read_cache_enable, "1") == 0);
    _index_cache_enabled = (index_cache_enable != NULL && std::strcmp(index_cache_enable, "1") == 0);
    _write_cache_enabled = (write_cache_enable != NULL && std::strcmp(write_cache_enable, "1") == 0);

    if (!_read_cache_enabled || read_ahead == NULL)
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

    Debug(std::cout << "Cache enabled: " << _read_cache_enabled << "\n";);
    Debug(std::cout << "Write component enabled: " << _write_cache_enabled << "\n";);
    Debug(std::cout << "Index cache enabled: " << _index_cache_enabled << "\n";);
    Debug(std::cout << "Cache readahead: " << _read_ahead << "\n";);
}

void CacheHandler::store_in_cache(void* src, size_t size, void* base_ptr, const std::vector<long>& initial_indices)
{
    if (_read_cache_enabled)
    {
        Cacheline value {src, size};
        auto key = std::make_pair(base_ptr, initial_indices);
        _read_cache.insert_or_assign(std::move(key), std::move(value));
    }
}

Cacheline* CacheHandler::find_cacheline(void* const base_ptr, const std::vector<long>& indices)
{
    if (_read_cache_enabled)
    {
        Cacheline* res = nullptr;
        auto entry = _read_cache.find({base_ptr, indices});
        if (entry != _read_cache.end())
        {
            res = &(entry->second);
        }
        return res;
    }
    return nullptr;
}

void CacheHandler::store_in_index_cache_local(void* target, size_t size, void* base_ptr, const std::vector<long>& initial_indices)
{
    if (_index_cache_enabled)
    {
        Indexline value {target, size};
        auto key = std::make_pair(base_ptr, initial_indices);
        _index_cache.insert({key, value});
    }
}

void CacheHandler::store_in_index_cache_remote(MemoryAbstraction* mem_abstraction, long index, void* base_ptr, const std::vector<long>& initial_indices)
{
    if (_index_cache_enabled)
    {
        Indexline value {mem_abstraction, index};
        auto key = std::make_pair(base_ptr, initial_indices);
        _index_cache.insert({key, value});
    }
}

Indexline* CacheHandler::find_index(void* const base_ptr, const std::vector<long>& indices)
{
    if (_index_cache_enabled)
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

void CacheHandler::drop_caches()
{
    clear_read_cache();
    clear_write_cache();
}

void CacheHandler::store_in_write_cache(void* data, int target_rank, long displacement, void* mem_abstraction_base, MPI_Datatype type, MPI_Win win)
{
    if (_write_cache_enabled)
    {
        auto map_entry = _write_cache.insert({mem_abstraction_base, {win, type}});
        Writecache& cache = map_entry.first->second;
        cache.insert_element(data, target_rank, displacement);
    }
}

void CacheHandler::clear_write_cache()
{
    if (_write_cache_enabled)
    {
        for (auto& elem : _write_cache)
        {
            elem.second.clear_cache();
        }
    }
}

void CacheHandler::clear_read_cache()
{
    _read_cache.clear();
}