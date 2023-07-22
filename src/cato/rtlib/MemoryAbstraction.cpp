/*
 * File: MemoryAbstraction.cpp
 * -----
 *
 * -----
 * Last Modified: Sat Jul 22 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 */

#include "MemoryAbstraction.h"

#include "../debug.h"
#include <iostream>
#include <stdlib.h>
#include <mpi.h>

MemoryAbstraction::MemoryAbstraction(long size, MPI_Datatype type, int dimensions)
{
    _base_ptr = nullptr;
    _size_bytes = size;
    _type = type;
    _dimensions = dimensions;
    MPI_Type_size(type, &_type_size);
}

MemoryAbstraction::~MemoryAbstraction() {}

/*void MemoryAbstraction::store(void *base_ptr, void *value_ptr, const std::vector<long> indices, Cache* cache, const std::vector<long>& initial_indices) {}

void MemoryAbstraction::load(void *base_ptr, void *dest_ptr, const std::vector<long> indices, Cache* cache, const std::vector<long>& initial_indices) {}

void MemoryAbstraction::sequential_store(void *base_ptr, void *value_ptr, const std::vector<long> indices){}

void MemoryAbstraction::sequential_load(void *base_ptr, void *dest_ptr, const std::vector<long> indices){}

void MemoryAbstraction::pointer_store(void *source_ptr, long dest_index) {}*/

void *MemoryAbstraction::get_base_ptr() { return _base_ptr; }

long MemoryAbstraction::get_size_bytes() { return _size_bytes; }

MPI_Datatype MemoryAbstraction::get_type() { return _type; }

int MemoryAbstraction::get_type_size() { return _type_size; }
