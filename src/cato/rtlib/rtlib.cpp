#include "rtlib.h"

#include <mpi.h>
#include <netcdf.h>
#include <netcdf_par.h>
#include <stdio.h>

#include <cstdarg>
#include <iostream>

#include "mpi_mutex.h"
// #include <fstream>
// #include <llvm/Support/raw_ostream.h>


void check_error_code(int err, const std::string &location)
{
    if (err)
    {
        std::cerr << "Error in " << location << " with error code: " << err << "\n";
    }
}

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


/**
 * omode:
 * 0: NC_NOWRITE
 */
int io_open(const char *path, int omode, int *ncidp)
{
    int err;
    if (omode == 0)
    {
        err = nc_open(path, NC_NOWRITE, ncidp);
    }
    else
    {
        // TODO: Implement more modes
        // llvm::errs() << "Mode " << omode << " has not been implemented within netCDF backend, yet\n";
        std::cerr << "Mode " << omode << " has not been implemented within netCDF backend, yet\n";
        err = 1;
    }

    check_error_code(43, "Hallo");
    Debug(check_error_code(err, "io_open (netCDF backend)"););
    return err;
}

// TODO: add MPI_Comm comm, MPI_Info info, to parameters
int io_open_par(const char *path, int omode, int *ncidp)
{
    int err;
    if (omode == 0)
    {
        
        err = nc_open_par(path, NC_NOWRITE, MPI_COMM_WORLD, MPI_INFO_NULL, ncidp);
    }
    else
    {
        // TODO: Implement more modes
        // llvm::errs() << "Mode " << omode << " has not been implemented within netCDF backend, yet\n";
        std::cerr << "Mode " << omode << " has not been implemented within netCDF backend, yet\n";
        err = 1;
    }

    Debug(check_error_code(err, "io_open_par (netCDF backend)"););
    return err;
}

int io_var_par_access(int ncid, int varid, int par_access)
{
    int err, mode;
    if (par_access == 0)
    {
        mode = NC_COLLECTIVE;
        Debug(errs() << "Use collective io mode for netCDF\n";);
    }
    else
    {
        mode = NC_INDEPENDENT;
        Debug(errs() << "Use independent io mode for netCDF\n";);
    }

    err = nc_var_par_access(ncid, varid, par_access);
    Debug(check_error_code(err, "io_var_par_access (netCDF backend)"););
    return err;
}

int io_inq_varid(int ncid, char *name, int *varidp)
{

    int err;
    err = nc_inq_varid(ncid, name, varidp);
    Debug(check_error_code(err, "io_inq_varid (netCDF backend)"););
    return err;
}

int io_get_var_int(int ncid, int varid, int *buffer)
{
    int err;

    err = nc_get_var_int(ncid, varid, buffer);
    Debug(check_error_code(err, "io_get_var_int (netCDF backend)"););
    return err;
}

// int io_get_vara_int(int ncid, int varid, const size_t *startp, const size_t *countp,
//                     int *buffer)
int io_get_vara_int(int ncid, int varid, int num_elements, int *buffer)
{
    int err;
    int rank = MPI_RANK;
    int size = MPI_SIZE;
    Debug(errs() << "Hello from rank " << rank << " (" << size << " total)\n";);

    size_t start, count;
    count = num_elements / size;
    if (rank < num_elements % size)
    {
        count += 1;
    }
    start = count * rank;
    if (rank >= num_elements % size)
    {
        start += num_elements % size;
    }

    Debug(errs() << "Rang "<< rank << ": Load distribution from " << start <<"\t with\t "<< count << "\t entries\n";);

    err = nc_get_vara_int(ncid, varid, &start, &count, buffer);
    Debug(check_error_code(err, "io_get_vara_int (netCDF backend)"););

    return err;
}

int io_close(int ncid)
{

    int err;

    err = nc_close(ncid);
    Debug(check_error_code(err, "io_close (netCDF backend)"););

    return err;
}
