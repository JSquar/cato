/*
 * File: IndexCacheElement.cpp
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Sun Sep 03 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#include "IndexCacheElement.h"

IndexCacheElement::IndexCacheElement(MemoryAbstraction* mem_abstraction, long index, bool local_data, void* target, size_t size)
        : _mem_abstraction{mem_abstraction}, _index{index}, _data_is_local{local_data}
{
    if (_data_is_local)
    {
        _data = target;
        _element_size = size;
    }
}

std::pair<MemoryAbstraction*, long> IndexCacheElement::get_components_for_memory_abstraction_access()
{
    return std::make_pair(_mem_abstraction, _index);
}

void IndexCacheElement::load_from_local_element(void* dest_ptr)
{
    if (!_data_is_local)
    {
        throw std::runtime_error("Data is not local\n");
    }
    std::memcpy(dest_ptr, _data, _element_size);
}

void IndexCacheElement::store_to_local_element(void* value_ptr)
{
    if (!_data_is_local)
    {
        throw std::runtime_error("Data is not local\n");
    }
    std::memcpy(_data, value_ptr, _element_size);
}
