/*
 * File: MemoryAbstractionDefault.h
 * -----
 *
 * -----
 * Last Modified: Thu Jul 20 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 */

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

    Cache _cache;

    /**
     * Traverses the MemoryAbstractions starting at memory_abstraction, pointed to by indices, down to the second-to-last-dimension.
     * This function and the next two are necessary to deal with v2-style array usage. Check get_target_of_operation in
     * MemoryAbstractionHandler.cpp for a detailed description.
     **/
    MemoryAbstraction* dereference_pointers(MemoryAbstraction* const memory_abstraction, const std::vector<long> indices);

    /**
     * Traverses the MemoryAbstractions at index 0 and gathers information regarding their element size.
     * For efficiency's sake, the lowest level MemoryAbstraction containing all the data points is returned as well.
     **/
    std::pair<std::vector<long>, MemoryAbstraction*> get_elements_per_dimension(MemoryAbstraction* memory_abstraction, size_t dimensions);

    /**
     * Calculates the new index on a one-dimensional array from the previous indices and the
     * elements that are supposed to be in each dimension.
     **/
    long calculate_new_index(const std::vector<long> indices, const std::vector<long> num_elements_in_dimension);

    /**
     * Determines the memory abstraction and the index at which the store/load operation should take place.
     **/
    std::pair<MemoryAbstraction*, long> get_target_of_operation(void* base_ptr, std::vector<long> indices);


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

    /**
     * See OpenMP memory model. Used to drop the cache at implied flushes
     **/
    void strong_flush();
};

#endif
