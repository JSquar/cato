#include "MemoryAbstraction.h"

#include "../debug.h"
#include <iostream>
#include <stdlib.h>

MemoryAbstraction::MemoryAbstraction(long size, MPI_Datatype type, int dimensions)
{
    _base_ptr = nullptr;
    _size_bytes = size;
    _type = type;
    _dimensions = dimensions;
}

MemoryAbstraction::~MemoryAbstraction() {}

void MemoryAbstraction::store(void *base_ptr, void *value_ptr, std::vector<long> indices) {}

void MemoryAbstraction::load(void *base_ptr, void *dest_ptr, std::vector<long> indices) {}

void MemoryAbstraction::sequential_store(void *base_ptr, void *value_ptr,
                                         std::vector<long> indices)
{
}

void MemoryAbstraction::sequential_load(void *base_ptr, void *dest_ptr,
                                        std::vector<long> indices)
{
}

void MemoryAbstraction::pointer_store(void *source_ptr, long dest_index) {}

void *MemoryAbstraction::get_base_ptr() { return _base_ptr; }

long MemoryAbstraction::get_size_bytes() { return _size_bytes; }

MPI_Datatype MemoryAbstraction::get_type() { return _type; }
