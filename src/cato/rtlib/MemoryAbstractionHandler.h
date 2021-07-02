#ifndef CATO_RTLIB_MEMORY_ABSTRACTION_HANDLER_H
#define CATO_RTLIB_MEMORY_ABSTRACTION_HANDLER_H

#include <map>
#include <memory>
#include <vector>

#include "MemoryAbstraction.h"
#include "MemoryAbstractionSingleValue.h"

/**
 * There is one instance of this class inserted into the compiled program.
 * This class manages all shared memory objects and provides an interface
 * to pass through instructions to each shared memory object.
 **/
class MemoryAbstractionHandler
{
  private:
    /**
     * The data structure that holds all shared memory objects.
     * The shared memory objects are identified by the value of their
     * allocation address.
     **/
    std::map<long, std::unique_ptr<MemoryAbstraction>> _memory_abstractions;

    std::map<long, std::unique_ptr<MemoryAbstractionSingleValue>> _single_value_abstractions;

    int _mpi_rank;
    int _mpi_size;

  public:
    MemoryAbstractionHandler(int rank, int size);

    ~MemoryAbstractionHandler(){};

    /**
     * Creates a new shared memory object and returns a pointer to the allocated memory.
     * Takes the size of the object in bytes and the type as MPI_Datatype
     **/
    void *create_memory(long size, MPI_Datatype type, int dimensions);

    /**
     * Deletes the shared memory object and frees all related memory
     **/
    void free_memory(void *base_ptr);

    /**
     * See MemoryAbstraction::store
     **/
    void store(void *base_ptr, void *value_ptr, std::vector<long> indices);

    /**
     * See MemoryAbstraction::load
     **/
    void load(void *base_ptr, void *dest_ptr, std::vector<long> indices);

    /**
     * See MemoryAbstraction::sequential_store
     **/
    void sequential_store(void *base_ptr, void *value_ptr, std::vector<long> indices);

    /**
     * See MemoryAbstraction::sequential_load
     **/
    void sequential_load(void *base_ptr, void *dest_ptr, std::vector<long> indices);

    /**
     * See MemoryAbstraction::pointer_store
     **/
    void pointer_store(void *dest_ptr, void *source_ptr, long dest_index);

    /**
     * Creates a MemoryAbstractionSingleValue for the given shared variable (base_ptr)
     * and adds it to _single_value_abstractions.
     **/
    void allocate_shared_value(void *base_ptr, MPI_Datatype type);

    /**
     * See MemoryAbstractionSingleValue::store
     **/
    void shared_value_store(void *base_ptr, void *value_ptr);

    /**
     * See MemoryAbstractionSingleValue::load
     **/
    void shared_value_load(void *base_ptr, void *dest_ptr);

    /**
     * See MemoryAbstractionSingleValue::synchronize
     **/
    void shared_value_synchronize(void *base_ptr);
};

#endif
