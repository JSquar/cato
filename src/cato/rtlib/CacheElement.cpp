/*
 * File: CacheElement.cpp
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Mon Aug 21 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */


#include "CacheElement.h"

#include <cstring>

CacheElement::CacheElement(void* data, size_t element_size)
    : _element_size{element_size}
{
    _data = std::malloc(element_size);
    if (_data == nullptr)
    {
        std::bad_alloc ex;
        throw ex;
    }
    std::memcpy(_data, data, element_size);
}

CacheElement::~CacheElement()
{
    std::free(_data);
}

CacheElement::CacheElement(const CacheElement& other)
    : _element_size{other._element_size}
{
    _data = std::malloc(_element_size);
    std::memcpy(_data, other._data, _element_size);
}

CacheElement& CacheElement::operator=(const CacheElement& other)
{
    CacheElement tmp {other};
    std::swap(tmp, *this);
    return *this;
}

CacheElement::CacheElement(CacheElement&& other)
    :  _data{other._data}, _element_size{other._element_size}
{
    other._element_size = 0;
    other._data = nullptr;
}

CacheElement& CacheElement::operator=(CacheElement&& other)
{
    std::swap(_element_size, other._element_size);
    std::swap(_data, other._data);
    return *this;
}