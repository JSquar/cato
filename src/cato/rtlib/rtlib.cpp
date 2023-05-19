/*
 * File: rtlib.cpp
 * 
 * -----
 * Last Modified: Friday, 19th May 2023 7:00:44 pm
 * Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * Copyright (c) 2019 Tim Jammer
 * Copyright (c) 2020 Michael Blesel
 * Copyright (c) 2023 Jannek Squar
 * 
 */
#include "rtlib.h"

#include <cstdlib>
#include <mpi.h>
#include <netcdf.h>
#include <netcdf_par.h>
#include <stdio.h>
#include <stdlib.h>

#include <cstdarg>
#include <iostream>
#include <optional>
#include "mpi_mutex.h"
#include <sys/stat.h>
#include "../environment_interaction.h"
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
    // TODO: IO Handler hier initialisieren?

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
    check_error_code(err, "io_open (netCDF backend)");
    return err;
}

// TODO: add MPI_Comm comm, MPI_Info info, to parameters
int io_open_par(const char *path, int omode, int *ncidp)
{
    int err;

    // set alignment
    std::size_t alignment = 4096;
    std::optional<std::size_t> env_alignment = parse_env_size_t("CATO_NC_ALIGNMENT");
    if(env_alignment.has_value()){
        alignment = env_alignment.value();
    }
    llvm::errs() << "Use alignment " << alignment << "\n";

    err = nc_set_alignment(0,alignment);

    if (omode == 0)
    {
        // std::cerr << "Pfad: " << path << "\n";   
        // std::cerr << "Omode: " << omode << "\n";   
        // std::cerr << "ncidp: " << ncidp << "\n";   
        // std::cerr << "MPI_COMM_WORLD: " << MPI_COMM_WORLD << "\n";   
        // std::cerr << "MPI_INFO_NULL: " << MPI_INFO_NULL << "\n";   
        struct stat buffer;   
        if (!(stat(path, &buffer) == 0)) {
            std::cerr << "Could not find file " << path << "\n";
        }
        err += nc_open_par(path, NC_NOWRITE, MPI_COMM_WORLD, MPI_INFO_NULL, ncidp);
        // err = nc_open(path, NC_NOWRITE, &test);

        // std::cerr << "Error value: " << err << "\n";
    }
    else
    {
        // TODO: Implement more modes
        // llvm::errs() << "Mode " << omode << " has not been implemented within netCDF backend, yet\n";
        std::cerr << "Mode " << omode << " has not been implemented within netCDF backend, yet\n";
        err = 1;
    }

    check_error_code(err, "io_open_par (netCDF backend)");
    return err;
}

int io_create_par(const char *path, int cmode, int *ncidp) {
    int err;

    // set alignment
    std::size_t alignment = 4096;
    std::optional<std::size_t> env_alignment = parse_env_size_t("CATO_NC_ALIGNMENT");
    if(env_alignment.has_value()){
        alignment = env_alignment.value();
    }
    llvm::errs() << "Use alignment " << alignment << "\n";

    err = nc_set_alignment(0,alignment);

    err += nc_create_par(path, cmode, MPI_COMM_WORLD, MPI_INFO_NULL, ncidp);
    check_error_code(err, "io_create_par (netCDF backend)");
    return err;
}

/**
 * Currently unused, since this logic has been inserted into io_inq_varid function
 */
int io_var_par_access(int ncid, int varid, int par_access)
{
    int err, mode;
    if (par_access == 0)
    {
        mode = NC_COLLECTIVE;
        Debug(llvm::errs() << "Use collective io mode for netCDF\n";);
    }
    else
    {
        mode = NC_INDEPENDENT;
        Debug(llvm::errs() << "Use independent io mode for netCDF\n";);
    }

    err = nc_var_par_access(ncid, varid, mode);
    check_error_code(err, "io_var_par_access (netCDF backend)");
    return err;
}

/**
 * Set collective or independent mode
 */
int io_inq_varid(int ncid, char *name, int *varidp)
{

    int err, nc_par_mode=NC_COLLECTIVE;
    
    err = nc_inq_varid(ncid, name, varidp);
    check_error_code(err, "io_inq_varid: inq varid (netCDF backend)"); //TODO

    if(const char* env_mode = std::getenv("CATO_NC_PAR_MODE")) {
        std::cerr << "Use " << env_mode << " mode\n";
        if(strcmp(env_mode, "COLLECTIVE") == 0) {
            nc_par_mode=NC_COLLECTIVE;
        }
        else if (strcmp(env_mode, "INDEPENDENT") == 0)
        {
            nc_par_mode=NC_INDEPENDENT;
        }
        else {
            std::cerr << "Could not recognise mode " << env_mode << ", make fallback on collective mode\n";
        }
    }
    else
    {
        std::cerr << "No par mode has been passed via CATO_PAR_MODE. CATO will use collective mode by default, you can choose between INDEPENDENT and COLLECTIVE\n";
    }
        
    err = nc_var_par_access(ncid, varid, nc_par_mode);
    check_error_code(err, "io_inq_varid: set par access mode (netCDF backend)"); //TODO

    return err;
}

int io_get_vara_int(int ncid, int varid, long int num_elements, int *buffer)
{
    int err;
    Debug(llvm::errs() << "Hello from rank " << MPI_RANK << " (" << MPI_SIZE << " total)\n";); //TODO

    size_t start, count;
    count = num_elements / MPI_SIZE / 4;
    if (MPI_RANK < num_elements % MPI_SIZE)
    {
        count += 1;
    }
    // count = 10;
    start = count * MPI_RANK;
    if (MPI_RANK >= num_elements % MPI_SIZE)
    {
        start += num_elements % MPI_SIZE;
    }

    // llvm::errs() << "Rang "<< MPI_RANK << ": Load distribution from " << start <<"\t with\t "<< count << "\t entries\n"; //TODO
    // int *buffer2 = (int*)malloc(sizeof(int) * 1073741824/4);

    // llvm::errs() << "Rang "<< MPI_RANK << ": ncid " << ncid << "\n";
    // llvm::errs() << "Rang "<< MPI_RANK << ": varid " << varid << "\n";
    // llvm::errs() << "Rang "<< MPI_RANK << ": start " << start << "\n";
    // llvm::errs() << "Rang "<< MPI_RANK << ": count " << count << "\n";
    // llvm::errs() << "Rang "<< MPI_RANK << ": buffer " << buffer << "\n";

    err = nc_get_vara_int(ncid, varid, &start, &count, buffer);
    check_error_code(err, "io_get_vara_int (netCDF backend)");

    return err;
}

int io_get_vara_float(int ncid, int varid, long int num_elements, float *buffer)
{
    int err;
    Debug(llvm::errs() << "Hello from rank " << MPI_RANK << " (" << MPI_SIZE << " total)\n";); //TODO

    size_t start, count;
    count = num_elements / MPI_SIZE / 4;
    if (MPI_RANK < num_elements % MPI_SIZE)
    {
        count += 1;
    }
    // count = 10;
    start = count * MPI_RANK;
    if (MPI_RANK >= num_elements % MPI_SIZE)
    {
        start += num_elements % MPI_SIZE;
    }

    err = nc_get_vara_float(ncid, varid, &start, &count, buffer);
    check_error_code(err, "io_get_vara_float (netCDF backend)");

    return err;
}

int io_close(int ncid)
{

    int err;

    err = nc_close(ncid);
    check_error_code(err, "io_close (netCDF backend)");

    return err;
}

int io_put_vara_int(int ncid, int varid, long int num_elements, int *buffer) {
    int err;
    size_t start, count;

    count = num_elements / MPI_SIZE / 4;
    if (MPI_RANK < num_elements % MPI_SIZE)
    {
        count += 1;
    }
    // count = 10;
    start = count * MPI_RANK;
    if (MPI_RANK >= num_elements % MPI_SIZE)
    {
        start += num_elements % MPI_SIZE;
    }

    err = nc_put_vara_int(ncid, varid, &start, &count, buffer);
    check_error_code(err, "io_put_vara_int (netCDF backend)"); 

    return err;
}

int io_put_vara_float(int ncid, int varid, long int num_elements, float *buffer) {
    int err;
    size_t start, count;

    count = num_elements / MPI_SIZE / 4;
    if (MPI_RANK < num_elements % MPI_SIZE)
    {
        count += 1;
    }
    // count = 10;
    start = count * MPI_RANK;
    if (MPI_RANK >= num_elements % MPI_SIZE)
    {
        start += num_elements % MPI_SIZE;
    }

    err = nc_put_vara_float(ncid, varid, &start, &count, buffer);
    check_error_code(err, "io_put_vara_int (netCDF backend)"); 

    llvm::errs() << "Rang "<< MPI_RANK << ": Load distribution from " << start <<"\t with\t "<< count << "\t entries\n"; //TODO
    int *buffer2 = (int*)malloc(sizeof(int) * 1073741824/4);

    llvm::errs() << "Rang "<< MPI_RANK << ": ncid " << ncid << "\n" << "Rang "<< MPI_RANK << ": varid " << varid << "\n" << "Rang "<< MPI_RANK << ": start " << start << "\n" << "Rang "<< MPI_RANK << ": count " << count << "\n" << "Rang "<< MPI_RANK << ": buffer " << buffer << "\n";

    return err;
}


int io_def_var(int ncid, const char *name, int xtype, int ndims, const int *dimidsp, int *varidp ) {
    int err = 0;

    err = nc_def_var(ncid, name, xtype, ndims, dimidsp, varidp);
    std::optional<std::size_t> chunking_value = parse_env_size_t("CATO_NC_CHUNKING");
    if (chunking_value.has_value())
    {
        std::cout<<"Set user-defined chunking\n";
        err += io_def_var_chunking(ncid, *varidp, NC_CHUNKED, xtype, chunking_value.value());
        err += io_set_compression(ncid, varid);
    }
    else {
        std::cout<<"Set no chunking\n";        
    }
    

    check_error_code(err, "io_def_var (netCDF backend)"); 
    return err;
}

int io_def_var_chunking(int ncid, int varid, int storage, int datatype, size_t bytes) {
    int err = 0;
    size_t chunksize[1] = {bytes}; //TODO allow multiple dimensions
    if(chunksize[0] > 4*1024*1024*1024L) {
        llvm::errs() << "Chunksize of " << chunksize[0] << " is too big, set chunk size to max value 4GiB\n";
        chunksize[0] = 4*1024*1024*1024L;
    }
    std::cout << "Use chunksize " << chunksize[0] << "\n";
    err = nc_def_var_chunking(ncid, varid, storage, chunksize);
    return err;   
}

int io_set_compression(int ncid, int varid) {
    int err = 0;

    // Check which compressor shall be used    
    if (std::getenv("CATO_NC_CMPR_DEFLATE"))
    {
        llvm::errs() << "Apply deflate compression on data\n";
        std::vector<std::string> deflate_values = parse_env_list("CATO_NC_CMPR_DEFLATE");
        if(deflate_values.size() > 2) {
            llvm::errs() << "CATO_NC_CMPR_DEFLATE should have at least one and at max two parameters. Skip compression step!\n";
            return 1;
        }
        else {
            int deflate_level = std::stoi(deflate_values.at(0));
            if(deflate_level < 0 || deflate_level > 9) {
                llvm::errs() << "Invalid deflate compression level " << deflate_level << "\n";
                return 1;
            }
            else {
                int shuffle = 0;
                if(deflate_values.size() == 2) {
                    shuffle = std::stoi(deflate_values.at(1));
                }

                    err = nc_def_var_deflate(ncid, varid, shuffle, 1, deflate_level);
                    if (err != NC_NOERR) {
                        fprintf(stderr, "Error setting compression filter deflate: %s\n", nc_strerror(err));
                        return err;
                    }
            }
        }
    }
    return err;
}