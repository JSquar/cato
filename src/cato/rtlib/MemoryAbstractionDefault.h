/*
 * File: MemoryAbstractionDefault.h
 * -----
 *
 * -----
 * Last Modified: Sat Jul 22 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 */
#ifndef CATO_RTLIB_MEMORY_ABSTRACTION_DEFAULT_H
#define CATO_RTLIB_MEMORY_ABSTRACTION_DEFAULT_H

#include <mpi.h>

#include <utility>
#include <vector>

#include "MemoryAbstraction.h"

/**
 * Default Communication Pattern for shared memory objects.
 * The elements of the shared memory are distributed evenly over all
 * mpi processes.
 * Communication is done through one-sided MPI.
 **/
class MemoryAbstractionDefault : public MemoryAbstraction
{
  private:
    // MPI window for the shared memory section
    MPI_Win _mpi_window;

    int _mpi_rank, _mpi_size;

    // global number of elements in the shared memory object and
    // the number of elements stored in the memory of this MPI process.
    long _global_num_elements, _local_num_elements;

    // Ranges of indices for the elements each MPI process has stored locally
    std::vector<std::pair<long, long>> _array_ranges;

    /**
     * Takes an offset and computes the rank of the MPI process that
     * stores the value at that offset. Also returns the local offset
     * of the searched element for the MPI process that stores it.
     **/
    std::pair<int, long> get_target_rank_and_disp_for_offset(long offset);

    /**
     * Allocate the actual memory on each MPI process and set up the MPI Window
     * and all needed variables for future communication.
     **/
    void create_1d_array(long size, MPI_Datatype type, int dimensions);

  public:
    /**
     * Create a MemeoryAbstraction of size (in bytes) with the given type
     * and dimensions.
     **/
    MemoryAbstractionDefault(long size, MPI_Datatype type, int dimensions);

    /**
     * Free all Memory and MPI Windows
     **/
    ~MemoryAbstractionDefault() override;

    /**
     * Stores the value at the address value_ptr into the memory Abstraction at
     * the given indices.
     **/
    void store(void *base_ptr, void *value_ptr, const std::vector<long> indices, Cache* cache, const std::vector<long>& initial_indices) override;

    /**
     * Loads the value at the given indices and copies it to the given dest_ptr address.
     **/
    void load(void *base_ptr, void *dest_ptr, const std::vector<long> indices, Cache* cache, const std::vector<long>& initial_indices) override;

    /**
     * Same as store but each process only continues after the store has been completed.
     **/
    void sequential_store(void *base_ptr, void *value_ptr, const std::vector<long> indices) override;

    /**
     * Same as load but each process only continues after the load has been completed.
     **/
    void sequential_load(void *base_ptr, void *dest_ptr, const std::vector<long> indices) override;

    /**
     * Stores the source_ptr into the memory abstraction at the given index.
     **/
    void pointer_store(void *source_ptr, long dest_index) override;
};

#endif
