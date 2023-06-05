#include "rtlib.h"

#include <cstdlib>
#include <mpi.h>
#include <netcdf.h>
#include <netcdf_filter.h>
#include <netcdf_par.h>
#include <stdio.h>
#include <stdlib.h>

#include <cstdarg>
#include <iostream>
#include <tuple>

#include <cstdarg>
#include <iostream>
#include <optional>
#include "mpi_mutex.h"
#include <sys/stat.h>
//#include "../environment_interaction.h"
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
    Debug(check_error_code(err, "io_open (netCDF backend)"););
    return err;
}

// TODO: add MPI_Comm comm, MPI_Info info, to parameters
int io_open_par(const char *path, int omode, int *ncidp)
{
    int err, test;
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
        err = nc_open_par(path, NC_NOWRITE, MPI_COMM_WORLD, MPI_INFO_NULL, ncidp);
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

    Debug(check_error_code(err, "io_open_par (netCDF backend)"););
    return err;
}

int io_create_par(const char *path, int cmode, int *ncidp) {
    int err;

    // set alignment
    std::size_t alignment = 4096;
    //TODO enable again
    /*std::optional<std::size_t> env_alignment = parse_env_size_t("CATO_NC_ALIGNMENT");
    if(env_alignment.has_value()){
        alignment = env_alignment.value();
    }
    llvm::errs() << "Use alignment " << alignment << "\n";*/

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

    err = nc_var_par_access(ncid, varid, par_access);
    Debug(check_error_code(err, "io_var_par_access (netCDF backend)"););
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

    if(const char* env_mode = std::getenv("CATO_PAR_MODE")) {
        //std::cerr << "Use " << env_mode << " mode\n";
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
    /*else
    {
        std::cerr << "No par mode has been passed via CATO_PAR_MODE. CATO will use collective mode by default, you can choose between INDEPENDENT and COLLECTIVE\n";
    }*/

    err = nc_var_par_access(ncid, varid, nc_par_mode);
    check_error_code(err, "io_inq_varid: set par access mode (netCDF backend)"); //TODO

    return err;
}

int io_get_var_int(int ncid, int varid, int *buffer)
{
    int err;

    err = nc_get_var_int(ncid, varid, buffer);
    Debug(check_error_code(err, "io_get_var_int (netCDF backend)"););
    return err;
}


//Turn the index in a one-dimensional array into an n-dimensional index representing that element
static void positionToIndex(const int num_dims, const size_t position, const size_t* dim_counts, size_t* index)
{
    size_t div_results[num_dims];
    index[num_dims - 1] = position % dim_counts[num_dims-1];
    div_results[num_dims - 1] = position / dim_counts[num_dims - 1];

    for (int i = num_dims - 2; i >= 0; i--)
    {
        index[i] = div_results[i+1] % dim_counts[i];
        div_results[i] = div_results[i+1] / dim_counts[i];
    }
}

//Determine the process-local section of the shared memory
//TODO mainly a copy-paste from MemoryAbstractionDefault
static std::tuple<size_t, size_t, size_t> determinePositionInSharedMem(long int num_bytes, size_t element_size)
{
    size_t global_num_elements = num_bytes / element_size;

    size_t div = global_num_elements / MPI_SIZE;
    size_t rest = global_num_elements % MPI_SIZE;

    size_t local_num_elements, local_from, local_to;

    if ((size_t)MPI_RANK < rest)
    {
        local_num_elements = div + 1;
        local_from = MPI_RANK * local_num_elements;
        local_to = local_from + local_num_elements - 1;
    }
    else
    {
        local_num_elements = div;
        local_from = MPI_RANK * local_num_elements + rest;
        local_to = local_from + local_num_elements - 1;
    }

    return std::make_tuple(local_num_elements, local_from, local_to);
}

int io_get_vara(int ncid, int varid, long int num_bytes, void *buffer, int nctype)
{
    int err;

    int num_dims;
    err = nc_inq_varndims(ncid, varid, &num_dims);
    check_error_code(err, "nc_inq_varndims (netCDF backend)");

    int dimids[num_dims];
    err = nc_inq_vardimid(ncid, varid, dimids);
    check_error_code(err, "nc_inq_vardimid (netCDF backend)");

    size_t count[num_dims];
    for(int i = 0; i < num_dims; i++)
    {
        err = nc_inq_dimlen(ncid, dimids[i], &count[i]);
        check_error_code(err, "nc_inq_dimlen (netCDF backend)");
    }

    size_t element_size = nctypelen(nctype);
    size_t local_num_elements, local_from, local_to;
    std::tie(local_num_elements, local_from, local_to) = determinePositionInSharedMem(num_bytes, element_size);

    size_t start_indexes[num_dims], end_indexes[num_dims];
    positionToIndex(num_dims, local_from, count, start_indexes);
    positionToIndex(num_dims, local_to, count, end_indexes);


    //In case that the reads per process do not align to rectangular hyperslabs,
    //we read slightly more than necessary to create these hyperslabs.
    //After that read, the correct number of elements is copied into the actual buffer.
    size_t read_indexes[num_dims];
    size_t read_counts[num_dims];

    read_indexes[0] = start_indexes[0];
    read_counts[0] = end_indexes[0] - start_indexes[0] + 1;
    size_t read_size = read_counts[0];

    for (int i = 1; i < num_dims; i++)
    {
        read_counts[i] = count[i];
        read_size *= count[i];
        read_indexes[i] = 0;
    }

    void* read_buf = malloc(element_size * read_size);

    //Read with modified boundaries, possibly more than necessary
    err = nc_get_vara(ncid, varid, read_indexes, read_counts, read_buf);
    std::string location = "io_get_vara (netCDF backend) with nctype: " + std::to_string(nctype);
    check_error_code(err, location); //TODO

    //Calculate the number of elements at the start that are too many
    size_t read_index_1d = 1;
    for (int i = 1; i < num_dims; i++)
    {
        read_index_1d *= count[i];
    }
    read_index_1d *= read_indexes[0];
    size_t offset_elements = local_from - read_index_1d;

    //Copy into the actual target
    memcpy(buffer, ((char*) read_buf) + offset_elements * element_size, local_num_elements * element_size);

    free(read_buf);
    return err;
}

int io_put_vara(int ncid, int varid, long int num_bytes, void *buffer, int nctype)
{
    int err;

    int num_dims;
    err = nc_inq_varndims(ncid, varid, &num_dims);
    check_error_code(err, "nc_inq_varndims (netCDF backend)");

    int dimids[num_dims];
    err = nc_inq_vardimid(ncid, varid, dimids);
    check_error_code(err, "nc_inq_vardimid (netCDF backend)");

    size_t count[num_dims];
    for(int i = 0; i < num_dims; i++)
    {
        err = nc_inq_dimlen(ncid, dimids[i], &count[i]);
        check_error_code(err, "nc_inq_dimlen (netCDF backend)");
    }

    size_t element_size = nctypelen(nctype);
    size_t local_num_elements, local_from;
    std::tie(local_num_elements, local_from, std::ignore) = determinePositionInSharedMem(num_bytes, element_size);

    size_t start_indexes[num_dims];
    positionToIndex(num_dims, local_from, count, start_indexes);

    size_t hyperslab_count[num_dims];
    size_t elements_left = local_num_elements;
    for (int i = num_dims-1; i >= 0; i--)
    {
        if (elements_left > count[i])
        {
            if (elements_left % count[i] != 0)
            {
                //TODO
                std::cerr << "Process count and nc file layout lead to irregular hyperslabs! This is currently not supported by CATO\n";
                exit(1);
            }
            hyperslab_count[i] = count[i];
            elements_left /= count[i];
        }
        else
        {
            hyperslab_count[i] = elements_left;
            for (int j = i-1; j >= 0; j--)
            hyperslab_count[j] = 1;
            break;
        }
    }

    err = nc_put_vara(ncid, varid, start_indexes, hyperslab_count, buffer);
    std::string location = "io_put_vara (netCDF backend) with nctype: " + std::to_string(nctype);
    check_error_code(err, location); //TODO

    return err;
}

int io_close(int ncid)
{

    int err;

    err = nc_close(ncid);
    Debug(check_error_code(err, "io_close (netCDF backend)"););

    return err;
}


int io_def_var(int ncid, const char *name, int xtype, int ndims, const int *dimidsp, int *varidp ) {
    int err = 0;

    err = nc_def_var(ncid, name, xtype, ndims, dimidsp, varidp);
    //std::optional<std::size_t> chunking_value = parse_env_size_t("CATO_NC_CHUNKING");
    //if (chunking_value.has_value())
    //TODO
    if(false)
    {
        std::cout<<"Set user-defined chunking\n";
        //err += io_def_var_chunking(ncid, *varidp, NC_CHUNKED, xtype, chunking_value.value());
        //err += io_set_compression(ncid, varid);
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

    /* -------------------------------- QUANTIZE -------------------------------- */
    if (std::getenv("CATO_NC_CMPR_QUANTIZE"))
    {
        llvm::errs() << "Apply lossy compression preprocessing on data\n";
        //std::vector<std::string> quantize_values = parse_env_list("CATO_NC_CMPR_QUANTIZE");
        //TODO
        std::vector<std::string> quantize_values{};
        if(quantize_values.size() != 2) {
            llvm::errs() << "CATO_NC_CMPR_QUANTIZE should have two parameters. Skip compression step!\n";
            return 1;
        }
        int quantize_mode = std::stoi(quantize_values.at(0));
        int quantize_nsd = std::stoi(quantize_values.at(1));

        if(quantize_mode == 0) {
            quantize_mode = NC_NOQUANTIZE;
            llvm::errs() << "Do not apply quantization preprocessing\n";
        }
        else {
            if(quantize_mode == 1) {
                quantize_mode = NC_QUANTIZE_BITGROOM;
            }
            else if (quantize_mode == 2)
            {
                quantize_mode = NC_QUANTIZE_GRANULARBR;
            }
            else if (quantize_mode == 3)
            {
                quantize_mode = NC_QUANTIZE_BITROUND;
            }
            else {
                llvm::errs() << "Unknown quantize mode: " << quantize_mode << "\n";
                return 1;
            }

            if(quantize_nsd < 1 ) {
                llvm::errs() << "Invalid number of significant digits ("<< quantize_nsd << ")for quantize\n";
                return 1;
            }

            err = nc_def_var_quantize(ncid, varid, quantize_mode, quantize_nsd);
            if (err != NC_NOERR) {
                fprintf(stderr, "Error setting lossy quantize preprocessing: %s\n", nc_strerror(err));
                return err;
            }
        }
    }

    /* --------------------------------- DEFLATE -------------------------------- */
    if (std::getenv("CATO_NC_CMPR_DEFLATE"))
    {
        llvm::errs() << "Apply deflate compression on data\n";
        //std::vector<std::string> deflate_values = parse_env_list("CATO_NC_CMPR_DEFLATE");
        //TODO
        std::vector<std::string> deflate_values{};
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

    /* ------------------------------ NETCDF FILTER ----------------------------- */
    if (std::getenv("CATO_NC_FILTER")) {
        //std::vector<unsigned int> filter_values = parse_env_list_int("CATO_NC_FILTER");
        //TODO
        std::vector<unsigned int> filter_values{};
        // unsigned int* parameters = &filter_values.data()[1];
        // llvm::errs() << "Apply filter data\nID: " << filter_values.at(0) << ", number parameter: " << filter_values.size() - 1 << "\n";
        // err = nc_def_var_filter(ncid, varid, filter_values.at(0) , filter_values.size() - 1, parameters);
        // unsigned int filterid = 307;
        // const unsigned int cd_values[1] = {9};  // Compression level

        // Check filter
        // err = nc_inq_filter_avail(ncid, filterid);
        // if (err != NC_NOERR) {
        //     fprintf(stderr, "Error checking filter %d availability: %s\n", filterid, nc_strerror(err));
        //     return err;
        // }

        // err = nc_def_var_filter(ncid, varid, filterid, 1, cd_values);

        err = nc_def_var_zstandard(ncid, varid, 9);

        if (err != NC_NOERR) {
            fprintf(stderr, "Error applying filter: %s\n", nc_strerror(err));
            return err;
        }

    }

    return err;
}