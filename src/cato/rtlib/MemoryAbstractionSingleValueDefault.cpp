#include "MemoryAbstractionSingleValueDefault.h"

#include <iostream>

#include "../debug.h"

MemoryAbstractionSingleValueDefault::MemoryAbstractionSingleValueDefault(void *base_ptr,
                                                                         MPI_Datatype type)
    : MemoryAbstractionSingleValue(base_ptr, type)
{
    int type_size;

    MPI_Comm_rank(MPI_COMM_WORLD, &_mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &_mpi_size);
    MPI_Type_size(type, &type_size);

    Debug(std::cout << "Trying to create a MPI_Window for a single value variable\n";);

    MPI_Win_create(_base_ptr, type_size, type_size, MPI_INFO_NULL, MPI_COMM_WORLD,
                   &_mpi_window);
}

MemoryAbstractionSingleValueDefault::~MemoryAbstractionSingleValueDefault()
{
    MPI_Win_free(&_mpi_window);
}

void MemoryAbstractionSingleValueDefault::store(void *base_ptr, void *value_ptr)
{
    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0, _mpi_window);
    MPI_Put(value_ptr, 1, _type, 0, 0, 1, _type, _mpi_window);
    MPI_Win_unlock(0, _mpi_window);
}

void MemoryAbstractionSingleValueDefault::load(void *base_ptr, void *dest_ptr)
{
    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0, _mpi_window);
    MPI_Get(dest_ptr, 1, _type, 0, 0, 1, _type, _mpi_window);
    MPI_Win_unlock(0, _mpi_window);
}

void MemoryAbstractionSingleValueDefault::synchronize(void *base_ptr)
{
    MPI_Barrier(MPI_COMM_WORLD);
    if (_mpi_rank != 0)
    {
        load(base_ptr, base_ptr);
    }
}
