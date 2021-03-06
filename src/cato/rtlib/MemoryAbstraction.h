#ifndef CATO_RTLIB_MEMORY_ABSTRACTION_H
#define CATO_RTLIB_MEMORY_ABSTRACTION_H

#include "../debug.h"
#include <mpi.h>
#include <vector>

/**
 * Base class for a shared memory object
 *
 * This class provides the basic interface that all communication patterns
 * should implement.
 **/
class MemoryAbstraction
{
  protected:
    void *_base_ptr;

    long _size_bytes;

    MPI_Datatype _type;

    int _dimensions;

  public:
    /**
     * Constructor needs the size of the allocated memory in bytes
     * and the type of the shared memory object as a MPI_Datatype
     *
     * The constructor is responsible for the actual memory allocation
     **/
    MemoryAbstraction(long size, MPI_Datatype type, int dimensions);

    /**
     * Destructor needs to free the allocated shared memory and
     * all related things (like MPI windows)
     **/
    virtual ~MemoryAbstraction();

    /**
     * A store to the shared memory object.
     * This gets called from prallelized sections of the original
     * program.
     **/
    virtual void store(void *base_ptr, void *value_ptr, std::vector<long> indices);

    /**
     * A load from the shared memory object.
     * This gets called from prallelized sections of the original
     * program.
     **/
    virtual void load(void *base_ptr, void *dest_ptr, std::vector<long> indices);

    /**
     * A store to the shared memory object.
     * This gets called from non OpenMP sections of the original program.
     * Depending on the communication pattern this function needs to make sure
     * that it can be called by all MPI processes simultaniously and that
     * this is a synchronous operation between all processes.
     **/
    virtual void sequential_store(void *base_ptr, void *value_ptr, std::vector<long> indices);

    /**
     * A load from the shared memory object.
     * This gets called from non OpenMP sections of the original program.
     * Depending on the communication pattern this function needs to make sure
     * that it can be called by all MPI processes simultaniously and that
     * this is a synchronous operation between all processes.
     **/
    virtual void sequential_load(void *base_ptr, void *dest_ptr, std::vector<long> indices);

    /**
     * A pointer store to an MemoryAbstraction with pointer depth >= 2.
     **/
    virtual void pointer_store(void *source_ptr, long dest_index);

    virtual void *get_base_ptr();

    virtual long get_size_bytes();

    virtual MPI_Datatype get_type();
};

#endif
