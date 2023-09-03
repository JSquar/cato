/*
 * File: IndexCacheElement.h
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Sun Sep 03 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */


#ifndef CATO_RTLIB_INDEXCACHEELEMENT_H
#define CATO_RTLIB_INDEXCACHEELEMENT_H

#include <cstdlib>
#include <optional>
#include "MemoryAbstraction.h"

/**
 * This class is used to as the element for the TLB-like index cache.
 * Whenever a certain index in a memory abstraction is accessed for the first time,
 * the usual procedure is to follow pointers and calculate new indices.
 * Since this is very time-consuming, this information is then cached in these elements.
 * If the data is local, the member functions get_data and get_size return the necessary
 * info to just copy the data to/from the correct address.
 * If the data is remote, the member functions get_memory_abstraction and get_index
 * return the necessary info to immediately call a load/store on the correct
 * memory abstraction instead of having to follow pointers around.
 **/
class IndexCacheElement
{
  private:
    void* _data;
    size_t _element_size;
    MemoryAbstraction* _mem_abstraction;
    long _index;
    bool _data_is_local;

    IndexCacheElement(MemoryAbstraction* mem_abstraction, long index, bool local_data, void* target, size_t size);
    
  public:
    static IndexCacheElement create_element_for_local_address(MemoryAbstraction* mem_abstraction, long index)
    {
        void* target_addr = mem_abstraction->get_address_of_local_element({index});
        return IndexCacheElement(mem_abstraction, index, true, target_addr, mem_abstraction->get_type_size());
    }
    static IndexCacheElement create_element_for_remote_address(MemoryAbstraction* mem_abstraction, long index)
    {
        return IndexCacheElement(mem_abstraction, index, false, nullptr, 0);
    }

    std::pair<MemoryAbstraction*, long> get_components_for_memory_abstraction_access();
    void load_from_local_element(void* dest_ptr);
    void store_to_local_element(void* value_ptr);

    bool is_data_local() const {return _data_is_local;}
    void* get_data() const {return _data;}
    size_t get_size() const {return _element_size;}
    long get_index() const {return _index;}
    MemoryAbstraction* get_memory_abstraction() const {return _mem_abstraction;}
};

#endif