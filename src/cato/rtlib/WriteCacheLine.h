/*
 * File: WriteCacheLine.h
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Mon Aug 14 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#ifndef CATO_RTLIB_WRITECACHELINE_H
#define CATO_RTLIB_WRITECACHELINE_H

#include <vector>
#include <mpi.h>
#include "CacheElement.h"

/**
 * Used as the elements for the write cache. A WriteCacheLine represents one MemoryAbstraction.
 * The data that is supposed to be written can be inserted one-by-one, accompanied by the target rank
 * and displacement. Upon clearing, the contents are then written to the appropriate targets.
 **/
class WriteCacheLine
{
  private:
    //This vector has one element per rank. The elements themselves are
    //also vectors, each storing pairs consisting of the target displacement
    //and the actual data in the form of a CacheElement.
    std::vector < std::vector <std::pair<long, CacheElement>> > _cached_elements;

    MPI_Win _win;

    MPI_Datatype _type;

    int _type_size;

    void clear_rank_vector(std::vector<std::pair<long, CacheElement>>& rank_vector, size_t rank);

  public:
    WriteCacheLine(MPI_Win win, MPI_Datatype type);

    void insert_element(void* data, int target_rank, long displacement, size_t flush_rank_after_num_elements);

    void clear_cache();
};

#endif