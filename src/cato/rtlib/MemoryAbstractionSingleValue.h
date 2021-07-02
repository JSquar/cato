#ifndef CATO_RTLIB_MEMORY_ABSTRACTION_SINGLE_VALUE_H
#define CATO_RTLIB_MEMORY_ABSTRACTION_SINGLE_VALUE_H

#include <mpi.h>

/**
 * Base class for MemoryAbstractions of single value shared variables in Microtasks.
 **/
class MemoryAbstractionSingleValue
{
  protected:
    void *_base_ptr;

    MPI_Datatype _type;

  public:
    /**
     * Constructor initializes private class variables.
     **/
    MemoryAbstractionSingleValue(void *base_ptr, MPI_Datatype type);

    /**
     * Virtual destructor.
     * Should free all Memory and MPI data structures that are used for this MemoryAbstraction.
     **/
    virtual ~MemoryAbstractionSingleValue();

    /**
     * This function has to be impplemented by all classes that inherit from this class.
     * Stores the value given at the address of value_ptr into the MemoryAbstraction
     **/
    virtual void store(void *base_ptr, void *value_ptr);

    /**
     * This function has to be impplemented by all classes that inherit from this class.
     * Loads the current value of the MemoryAbstraction and copies it to the dest_ptr address.
     **/
    virtual void load(void *base_ptr, void *dest_ptr);

    /**
     * Synchronizes all local versions of the shared variable for each MPI process.
     * This should be used at the end of a Microtask to make sure that all processes have the
     *same value for the single value shared variable after leaving a parallel section.
     **/
    virtual void synchronize(void *base_ptr);

    virtual void *get_base_ptr();

    virtual MPI_Datatype get_type();
};

#endif
