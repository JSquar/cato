/*
 * File: IndexCacheElement.cpp
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Sat Sep 02 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#include "IndexCacheElement.h"

std::pair<MemoryAbstraction*, long> IndexCacheElement::get_components_for_memory_abstraction_access()
{
    return std::make_pair(_mem_abstraction, _index);
}

void IndexCacheElement::load_from_local_element(void* dest_ptr)
{
    std::memcpy(dest_ptr, _data, _element_size);
}

void IndexCacheElement::store_to_local_element(void* value_ptr)
{
    std::memcpy(_data, value_ptr, _element_size);
}
