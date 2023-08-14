/*
 * File: CacheHandler.cpp
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Mon Aug 14 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#include <iostream>
#include <cstdlib>

#include "CacheHandler.h"
#include "../debug.h"

CacheHandler::CacheHandler()
    : _read_cache{is_enabled("CATO_ENABLE_READ_CACHE")}, _index_cache{is_enabled("CATO_ENABLE_INDEX_CACHE")},
      _write_cache{is_enabled("CATO_ENABLE_WRITE_CACHE"), env_var_value("CATO_FLUSH_WRITE_AFTER")}
{
    if (_read_cache.cache_enabled())
    {
        _read_ahead = env_var_value("CATO_CACHE_READAHEAD");
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