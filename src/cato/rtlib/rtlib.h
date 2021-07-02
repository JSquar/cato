#ifndef CATO_RTLIB_RTLIB_H
#define CATO_RTLIB_RTLIB_H

#include <iostream>
#include <memory>
#include <mpi.h>

#include "CatoRuntimeLogger.h"
#include "MemoryAbstractionHandler.h"

/**
 * Enum from llvm::AtomicRMWInst::BinOp
 * including the llvm header for this did not work
 * so this a local copy
 * link to definition:
 * https://llvm.org/doxygen/Instructions_8h_source.html#l00711
 **/
enum BinOp
{
    /// *p = v
    Xchg,
    /// *p = old + v
    Add,
    /// *p = old - v
    Sub,
    /// *p = old & v
    And,
    /// *p = ~(old & v)
    Nand,
    /// *p = old | v
    Or,
    /// *p = old ^ v
    Xor,
    /// *p = old >signed v ? old : v
    Max,
    /// *p = old <signed v ? old : v
    Min,
    /// *p = old >unsigned v ? old : v
    UMax,
    /// *p = old <unsigned v ? old : v
    UMin,

    /// *p = old + v
    FAdd,

    /// *p = old - v
    FSub,

    FIRST_BINOP = Xchg,
    LAST_BINOP = FSub,
    BAD_BINOP
};

/**
 * This header provides all functions that can be inserted into the pass.
 **/

// Global Variables
int MPI_RANK = 0;
int MPI_SIZE = 0;

// One global instance of the MemoryAbstractionHandler class to manage shared memory objects
std::unique_ptr<MemoryAbstractionHandler> _memory_handler;

/**
 * Dummy function for testing
 **/
void print_hello();

/**
 * Function for testing inserted code
 **/
void test_func(int num_args, ...);

/**
 * Initialize MPI and rtlib functionality
 **/
void cato_initialize(bool logging);

/**
 * Finalize MPI and clean up rtlib functionality
 **/
void cato_finalize();

/**
 * Returns the MPI rank of current process
 **/
int get_mpi_rank();

/**
 * Returns the number of MPI processes
 **/
int get_mpi_size();

/**
 * Insert MPI_Barrier call
 **/
void mpi_barrier();

/**
 * Allocate a shared memory segment
 **/
void *allocate_shared_memory(long size, MPI_Datatype, int dimensions);

/**
 * Free shared memory segment
 **/
void shared_memory_free(void *base_ptr);

/**
 * Store to a shared memory segment
 * Takes the base pointer of the shared memory object,
 * a void pointer to the value that is to be stored,
 * number of pointer access indices,
 * a list of access indices
 **/
void shared_memory_store(void *base_ptr, void *value_ptr, int num_indices, ...);

/**
 * Load from a shared memory segment
 * Takes the base pointer of the shared memory object,
 * a void pointer to the value that is to be stored,
 * number of pointer access indices,
 * a list of access indices
 **/
void shared_memory_load(void *base_ptr, void *dest_ptr, int num_indices, ...);

/**
 * Store in a non OpenMP section of the original program
 * Takes the base pointer of the shared memory object,
 * a void pointer to the value that is to be stored,
 * number of pointer access indices,
 * a list of access indices
 **/
void shared_memory_sequential_store(void *base_ptr, void *value_ptr, int num_indices, ...);

/**
 * Load in a non OpenMP section of the original program
 * Takes the base pointer of the shared memory object,
 * a void pointer to the value that is to be stored,
 * number of pointer access indices,
 * a list of access indices
 **/
void shared_memory_sequential_load(void *base_ptr, void *dest_ptr, int num_indices, ...);

/**
 * Store the pointer source_ptr into the MemoryAbstraction (dest_ptr) at the given index.
 **/
void shared_memory_pointer_store(void *dest_ptr, void *source_ptr, long dest_index);

/**
 * Create a MemoryAbstractionSingleValue for a single value shared variable inside a Microtask.
 **/
void allocate_shared_value(void *base_ptr, MPI_Datatype type);

/**
 * Store the value at the address value_ptr into the MemoryAbstractionSingleValue (base_ptr).
 **/
void shared_value_store(void *base_ptr, void *value_ptr);

/**
 * Load the value of the MemoryAbstrationSingleValue (base_ptr) and copy it to the address
 *dest_ptr.
 **/
void shared_value_load(void *base_ptr, void *dest_ptr);

/**
 * Synchronize the current correct value of the MemoryAbstractionSingleValue (base_ptr) to all
 *MPI processes. This should be called for each MemoryAbstractionSingleValue at the end of a
 *Microtask.
 **/
void shared_value_synchronize(void *base_ptr);

/**
 * Takes pointer to the upper/lower bound of a pragma omp parallel for loop and
 * modifies the values for each mpi process.
 **/
void modify_parallel_for_bounds(int *lower_bound, int *upper_bound, int increment);
void modify_parallel_for_bounds(long *lower_bound, long *upper_bound, long increment);

template <typename T>
void modify_parallel_for_bounds(T *lower_bound, T *upper_bound, T increment)
{
    Debug(std::cout << "Modifing parallel for loop bounds.\n";);
    Debug(std::cout << "Lower bound: " << *lower_bound << "\nUpper bound: " << *upper_bound
                    << "\nIncrement: " << increment << "\n";);

    if (increment != 1)
    {
        std::cerr << "Warning: Currently only loop increments of value 1 are supported\n";
    }

    T local_lbound, local_ubound;

    T total_iterations = *upper_bound - *lower_bound + 1;
    T div = total_iterations / MPI_SIZE;
    T rest = total_iterations % MPI_SIZE;

    if (MPI_RANK < rest)
    {
        local_lbound = *lower_bound + MPI_RANK * (div + 1);
        local_ubound = local_lbound + div;
    }
    else
    {
        local_lbound = *lower_bound + MPI_RANK * div + rest;
        local_ubound = local_lbound + div - 1;
    }

    Debug(std::cout << "Local lower bound: " << local_lbound << "\nLocal upper bound: "
                    << local_ubound << "\nIncrement: " << increment << "\n";);

    *lower_bound = local_lbound;
    *upper_bound = local_ubound;
}

/**
 * Creates a MPI mutex for this critical section
 **/
void *critical_section_init();

/**
 * The process tries to acquire the mutex.
 * If the mutex is not free the process waits until it can enter the critical section.
 **/
void critical_section_enter(void *mpi_mutex);

/**
 * The process releases the mutex
 **/
void critical_section_leave(void *mpi_mutex);

/**
 * Destroys the mutex
 **/
void critical_section_finalize(void *mpi_mutex);

/**
 * Performs the given reduction operation on the given values.
 * This is used to get a reduction result for the local reduction variables
 * of the processes.
 * The Pass itself still needs to combine this result with the initial value of
 * the shared variable that is reduced
 **/
void reduce_local_vars(void *local_var, int bin_op, MPI_Datatype type);

#endif
