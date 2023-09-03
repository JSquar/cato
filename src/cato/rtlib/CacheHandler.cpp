/*
 * File: CacheHandler.cpp
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Sun Sep 03 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#include <iostream>
#include <cstdlib>

#include "CacheHandler.h"
#include "../debug.h"
#include "PerformanceMetrics.h"

CacheHandler::CacheHandler()
    : _read_cache{is_enabled("CATO_ENABLE_READ_CACHE")}, _index_cache{is_enabled("CATO_ENABLE_INDEX_CACHE")},
      _write_cache{is_enabled("CATO_ENABLE_WRITE_CACHE"), env_var_value("CATO_FLUSH_WRITE_AFTER")}
{
    if (_read_cache.cache_enabled())
    {
        _read_ahead = static_cast<int>(env_var_value("CATO_CACHE_READAHEAD"));
    }
    else
    {
        _read_ahead = 0;
    }

    Debug(std::cout << "Cache enabled: " << _read_cache.cache_enabled() << "\n";);
    Debug(std::cout << "Write component enabled: " << _write_cache.cache_enabled() << "\n";);
    Debug(std::cout << "Index cache enabled: " << _index_cache.cache_enabled() << "\n";);
    Debug(std::cout << "Cache readahead: " << _read_ahead << "\n";);
}

bool CacheHandler::is_enabled(const char* env_var)
{
    char* env_var_content = std::getenv(env_var);
    return (env_var_content != NULL && std::strcmp(env_var_content, "1") == 0);
}

int CacheHandler::env_var_value(const char* env_var)
{
    int res = 0;
    char* env_var_content = std::getenv(env_var);

    if (env_var_content != NULL)
    {
        char* end;
        res = std::strtol(env_var_content, &end, 0);
        if (*end)
        std::cerr << "Could not read value for CATO_CACHE_READAHEAD";
    }

    return res;
}

void CacheHandler::store_in_index_cache(void* base_ptr, const std::vector<long>& initial_indices,
                                        MemoryAbstraction* mem_abstraction, const std::vector<long> indices)
{
    auto key = make_cache_key(base_ptr, initial_indices);
    if (mem_abstraction->is_data_local(indices))
    {
        IndexCacheElement local_value = IndexCacheElement::create_element_for_local_address(mem_abstraction, indices[0]);
        _index_cache.store_in_cache(key, local_value);
    }
    else
    {
        IndexCacheElement remote_value = IndexCacheElement::create_element_for_remote_address(mem_abstraction, indices[0]);
        _index_cache.store_in_cache(key, remote_value);
    }
}

void CacheHandler::store_in_read_cache(void* base_ptr, const std::vector<long>& initial_indices, void* dest_ptr, size_t element_size)
{
    auto key = make_cache_key(base_ptr, initial_indices);
    CacheElement read_value {dest_ptr, element_size};
    _read_cache.store_in_cache(key, read_value);
}

void CacheHandler::store_in_write_cache(void* base_ptr, MPI_Datatype type, MPI_Win win, void* value_ptr, int target_rank, long element_displacement)
{
    _write_cache.store_in_cache(base_ptr, type, win, value_ptr, target_rank, element_displacement);
}

CacheElement* CacheHandler::check_read_cache(void* base_ptr, const std::vector<long>& indices)
{
    auto read_cache_key = make_cache_key(base_ptr, indices);
    auto res = _read_cache.find_element(read_cache_key);
    if (res != nullptr)
    {
        cache_hit(CACHETYPE::READ);
    }
    else
    {
        cache_miss(CACHETYPE::READ);
    }
    return res;
}

IndexCacheElement* CacheHandler::check_index_cache(void* base_ptr, const std::vector<long>& indices)
{
    auto index_cache_key = make_cache_key(base_ptr, indices);
    auto res = _index_cache.find_element(index_cache_key);
    if (res != nullptr)
    {
        cache_hit(CACHETYPE::INDEX);
    }
    else
    {
        cache_miss(CACHETYPE::INDEX);
    }
    return res;
}

CacheHandler::CatoCacheKey CacheHandler::make_cache_key(void* const base_ptr, const std::vector<long>& indices)
{
    return {base_ptr, indices};
}

void CacheHandler::clear_write_cache()
{
    _write_cache.clear_cache();
}

void CacheHandler::clear_read_cache()
{
    _read_cache.clear_cache();
}