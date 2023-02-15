/*
 * File: rtlib_io.h
 * Created: Wednesday, 15th February 2023 6:40:32 pm
 * Author: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 *
 * -----
 * Last Modified: Wednesday, 15th February 2023 6:56:20 pm
 * Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Jannek Squar
 *
 */
#ifndef RTLIB_IO
#define RTLIB_IO

#include <memory>
#include <mpi.h>

/**
 * This header provides all netCDF functions that can be inserted into the pass.
 **/

// Global Variables
// TODO: ncid and varid should not be global variables
int ncid;
int varid;

// One global instance of the IOAbstractionHandler class to manage (parallel) IO and
// compression
std::unique_ptr<IOAbstractionHandler> _io_handler;

int io_open(const char *, int, int *);

int io_open_par(const char *, int, MPI_Comm, MPI_Info, int *);

int io_var_par_access(int, int, int);

int io_inq_varid(int, char *, int *);

int io_get_var_int(int, int, int *);

int io_get_vara_int(int, int, const size_t *, const size_t *, int *);

int io_close(int);

#endif /* RTLIB_IO */
