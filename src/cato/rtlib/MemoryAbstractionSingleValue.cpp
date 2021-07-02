#include "MemoryAbstractionSingleValue.h"

#include <iostream>

MemoryAbstractionSingleValue::MemoryAbstractionSingleValue(void *base_ptr, MPI_Datatype type)
{
    _base_ptr = base_ptr;
    _type = type;
}

MemoryAbstractionSingleValue::~MemoryAbstractionSingleValue() {}

void MemoryAbstractionSingleValue::store(void *base_ptr, void *value_ptr) {}

void MemoryAbstractionSingleValue::load(void *base_ptr, void *dest_ptr) {}

void MemoryAbstractionSingleValue::synchronize(void *base_ptr) {}

void *MemoryAbstractionSingleValue::get_base_ptr() { return _base_ptr; }

MPI_Datatype MemoryAbstractionSingleValue::get_type() { return _type; }
