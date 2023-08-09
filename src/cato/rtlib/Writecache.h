/*
 * File: Writecache.h
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Wed Aug 09 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#ifndef CATO_RTLIB_WRITECACHE_H
#define CATO_RTLIB_WRITECACHE_H

#include <vector>
#include <mpi.h>
#include "Cacheline.h"

class Writecache
{
  private:
    std::vector < std::vector <std::pair<long, Cacheline>> > _cached_elements;

    MPI_Win _win;

    MPI_Datatype _type;

    int _type_size;

  public:
    Writecache(MPI_Win win, MPI_Datatype type);

    void insert_element(void* data, int target_rank, long displacement);

    void clear_cache();
};

#endif