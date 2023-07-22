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

void *MemoryAbstraction::get_base_ptr() { return _base_ptr; }

long MemoryAbstraction::get_size_bytes() { return _size_bytes; }

MPI_Datatype MemoryAbstraction::get_type() { return _type; }

int MemoryAbstraction::get_type_size() { return _type_size; }
