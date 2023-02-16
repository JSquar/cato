/*
 * File: rtlib_io.cpp
 * Created: Wednesday, 15th February 2023 6:40:27 pm
 * Author: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 *
 * -----
 * Last Modified: Thursday, 16th February 2023 10:55:56 am
 * Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Jannek Squar
 *
 */

#include "../debug.h"
#include "../rtlib/rtlib.h"
#include <memory>
#include <mpi.h>
#include <netcdf.h>
#include <netcdf_par.h>

using namespace llvm;


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
        errs() << "Mode " << omode << " has not been implemented within netCDF backend, yet\n";
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
        errs() << "Mode " << omode << " has not been implemented within netCDF backend, yet\n";
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
