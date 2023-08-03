/*
 * File: Indexline.h
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Thu Aug 03 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */


#ifndef CATO_RTLIB_INDEXLINE_H
#define CATO_RTLIB_INDEXLINE_H

#include <cstdlib>
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
class Indexline
{
  private:
    void* _target;
    size_t _element_size;
    MemoryAbstraction* _mem_abstraction;
    long _index;
    bool _local_data;

  public:
    Indexline(void* target, size_t size) : _target{target}, _element_size{size}, _local_data{true}{};
    Indexline(MemoryAbstraction* mem_abstraction, long index) : _mem_abstraction{mem_abstraction}, _index{index}, _local_data{false}{};

    bool is_data_local() const {return _local_data;}
    void* get_data() const {return _target;}
    size_t get_size() const {return _element_size;}
    long get_index() const {return _index;}
    MemoryAbstraction* get_memory_abstraction() const {return _mem_abstraction;}
};

#endif