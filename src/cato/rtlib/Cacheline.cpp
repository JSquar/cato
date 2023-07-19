/*
 * File: Cacheline.cpp
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Wed Jul 19 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */


#include "Cacheline.h"

#include <cstring>

Cacheline::Cacheline(void* data, size_t element_size)
    : _element_size{element_size}
{
    _data = std::malloc(element_size);
    std::memcpy(_data, data, element_size);
}

Cacheline::~Cacheline()
{
    std::free(_data);
}

Cacheline::Cacheline(const Cacheline& other)
    : _element_size{other._element_size}
{
    _data = std::malloc(_element_size);
    std::memcpy(_data, other._data, _element_size);
}

Cacheline& Cacheline::operator=(const Cacheline& other)
{
    Cacheline tmp {other};
    std::swap(tmp, *this);
    return *this;
}

Cacheline::Cacheline(Cacheline&& other)
    :  _data{other._data}, _element_size{other._element_size}
{
    other._element_size = 0;
    other._data = nullptr;
}

Cacheline& Cacheline::operator=(Cacheline&& other)
{
    std::swap(_element_size, other._element_size);
    std::swap(_data, other._data);
    return *this;
}