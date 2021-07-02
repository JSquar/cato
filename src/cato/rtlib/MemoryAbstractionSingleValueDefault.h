#ifndef CATO_RTLIB_MEMORY_ABSTRACTION_SINGLE_VALUE_DEFAULT_H
#define CATO_RTLIB_MEMORY_ABSTRACTION_SINGLE_VALUE_DEFAULT_H

#include "MemoryAbstractionSingleValue.h"

/**
 * The default implementation for single value shared variables in Microtasks.
 *
 * A MPI window is created for the shared variable and MPI processes use one-sided
 *communication to always store/load the current value to/from the rank 0 process. At the end
 *of a Microtask the synchronize function neeeds to be called to give all processes the same
 *version of the shared variable before exiting the parallel section.
 **/
class MemoryAbstractionSingleValueDefault : public MemoryAbstractionSingleValue
{
  private:
    MPI_Win _mpi_window;

    int _mpi_rank, _mpi_size;

  public:
    /**
     * Create the MPI Window for the shared variable at the address base_ptr.
     **/
    MemoryAbstractionSingleValueDefault(void *base_ptr, MPI_Datatype type);

    /**
     * Free the MPI Window.
     **/
    ~MemoryAbstractionSingleValueDefault() override;

    /**
     * Store the value at address value_ptr into the rank 0 processe's version of the shared
     *variable.
     **/
    void store(void *base_ptr, void *value_ptr) override;

    /**
     * Load the current value of the shared variable from the rank 0 process and copy it to the
     *dest_ptr address.
     **/
    void load(void *base_ptr, void *dest_ptr) override;

    /**
     * Copy the current value of the shared variable from rank 0 to all other processes.
     **/
    void synchronize(void *base_ptr) override;
};

#endif
