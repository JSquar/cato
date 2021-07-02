#include "rtlib.h"

#include <mpi.h>
#include <stdio.h>

#include <cstdarg>
#include <iostream>

#include "mpi_mutex.h"
#include <fstream>

#include "../debug.h"

void print_hello() { std::cout << "HELLO\n"; }

void test_func(int num_args, ...) {}

void cato_initialize(bool logging)
{
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &MPI_RANK);
    MPI_Comm_size(MPI_COMM_WORLD, &MPI_SIZE);

    MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

    _memory_handler = std::make_unique<MemoryAbstractionHandler>(MPI_RANK, MPI_SIZE);

    if (logging)
    {
        CatoRuntimeLogger::start_logger();
    }
}

void cato_finalize()
{
    _memory_handler.reset();

    CatoRuntimeLogger::stop_logger();

    MPI_Finalize();
}

int get_mpi_rank() { return MPI_RANK; }

int get_mpi_size() { return MPI_SIZE; }

void mpi_barrier() { MPI_Barrier(MPI_COMM_WORLD); }

void *allocate_shared_memory(long size, MPI_Datatype type, int dimensions)
{
    return _memory_handler->create_memory(size, type, dimensions);
}

void shared_memory_free(void *base_ptr) { _memory_handler->free_memory(base_ptr); }

void shared_memory_store(void *base_ptr, void *value_ptr, int num_indices, ...)
{
    std::vector<long> indices;

    // Read the pointer access indices
    va_list ap;
    va_start(ap, num_indices);
    for (int i = 0; i < num_indices; i++)
    {
        indices.push_back(va_arg(ap, long));
    }
    va_end(ap);

    _memory_handler->store(base_ptr, value_ptr, indices);
}

void shared_memory_load(void *base_ptr, void *dest_ptr, int num_indices, ...)
{
    std::vector<long> indices;

    // Read the pointer access indices
    va_list ap;
    va_start(ap, num_indices);
    for (int i = 0; i < num_indices; i++)
    {
        indices.push_back(va_arg(ap, long));
    }
    va_end(ap);

    _memory_handler->load(base_ptr, dest_ptr, indices);
}

void shared_memory_sequential_store(void *base_ptr, void *value_ptr, int num_indices, ...)
{
    std::vector<long> indices;

    // Read the pointer access indices
    va_list ap;
    va_start(ap, num_indices);
    for (int i = 0; i < num_indices; i++)
    {
        indices.push_back(va_arg(ap, long));
    }
    va_end(ap);

    _memory_handler->sequential_store(base_ptr, value_ptr, indices);
}

void shared_memory_sequential_load(void *base_ptr, void *dest_ptr, int num_indices, ...)
{
    std::vector<long> indices;

    // Read the pointer access indices
    va_list ap;
    va_start(ap, num_indices);
    for (int i = 0; i < num_indices; i++)
    {
        indices.push_back(va_arg(ap, long));
    }
    va_end(ap);

    _memory_handler->sequential_load(base_ptr, dest_ptr, indices);
}

void shared_memory_pointer_store(void *dest_ptr, void *source_ptr, long dest_index)
{
    _memory_handler->pointer_store(dest_ptr, source_ptr, dest_index);
}

void allocate_shared_value(void *base_ptr, MPI_Datatype type)
{
    _memory_handler->allocate_shared_value(base_ptr, type);
}

void shared_value_store(void *base_ptr, void *value_ptr)
{
    _memory_handler->shared_value_store(base_ptr, value_ptr);
}

void shared_value_load(void *base_ptr, void *dest_ptr)
{
    _memory_handler->shared_value_load(base_ptr, dest_ptr);
}

void shared_value_synchronize(void *base_ptr)
{
    _memory_handler->shared_value_synchronize(base_ptr);
}

void modify_parallel_for_bounds(int *lower_bound, int *upper_bound, int increment)
{
    modify_parallel_for_bounds<int>(lower_bound, upper_bound, increment);
}

void modify_parallel_for_bounds(long *lower_bound, long *upper_bound, long increment)
{
    modify_parallel_for_bounds<long>(lower_bound, upper_bound, increment);
}

void *critical_section_init()
{
    MPI_Mutex *mutex = nullptr;
    MPI_Mutex_init(&mutex, 0);
    return (void *)mutex;
}

void critical_section_enter(void *mpi_mutex)
{
    MPI_Mutex *mutex = (MPI_Mutex *)mpi_mutex;
    MPI_Mutex_lock(mutex);
}

void critical_section_leave(void *mpi_mutex)
{
    MPI_Mutex *mutex = (MPI_Mutex *)mpi_mutex;
    MPI_Mutex_unlock(mutex);
}

void critical_section_finalize(void *mpi_mutex)
{
    MPI_Mutex *mutex = (MPI_Mutex *)mpi_mutex;
    MPI_Mutex_destroy(mutex);
    delete mutex;
}

void reduce_local_vars(void *local_var, int bin_op, MPI_Datatype type)
{
    switch (bin_op)
    {
    case BinOp::Add:
        MPI_Allreduce(MPI_IN_PLACE, local_var, 1, type, MPI_SUM, MPI_COMM_WORLD);
        break;
    case BinOp::Max:
        MPI_Allreduce(MPI_IN_PLACE, local_var, 1, type, MPI_MAX, MPI_COMM_WORLD);
        break;
    case BinOp::Min:
        MPI_Allreduce(MPI_IN_PLACE, local_var, 1, type, MPI_MIN, MPI_COMM_WORLD);
        break;
    default:
        std::cerr << "Error: unknown reduction operation\n";
        break;
    }
}
