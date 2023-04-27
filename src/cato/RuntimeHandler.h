/*
 * File: RuntimeHandler.h
 * -----
 *
 * -----
 * Last Modified: Wednesday, 26th April 2023 9:42:17 am
 * Modified By: Jannek Squar (jannek.squar@uni-hamburg.de)
 * -----
 * Copyright (c) 2020 Michael Blesel
 * Copyright (c) 2019 Tim Jammer
 * Copyright (c) 2023 Jannek Squar
 *
 */
#ifndef CATO_RUNTIME_HANDLER_H
#define CATO_RUNTIME_HANDLER_H

#include <llvm/ADT/StringRef.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Module.h>

#include <memory>

/**
 * All rtlib functions that can be called from inside the pass
 *
 * To add a new function that is usable by the pass do the following:
 *  1. Add the new rtlib function to rtlib.h/.c
 *  2. Add a new llvm::Function* to this struct
 *  3. Initialize the new function in RuntimeHandler::load_external_functions
 *      with match_function()
 *  4. To easily get the mangled function name for match_function use the
 *      scripts/get_rtlib_function_name.sh script
 **/
struct external_functions
{
    // MPI library functions
    llvm::Function *print_hello;
    llvm::Function *test_func;
    llvm::Function *cato_initialize;
    llvm::Function *cato_finalize;
    llvm::Function *get_mpi_rank;
    llvm::Function *get_mpi_size;
    llvm::Function *mpi_barrier;
    llvm::Function *allocate_shared_memory;
    llvm::Function *shared_memory_load;
    llvm::Function *shared_memory_store;
    llvm::Function *shared_memory_free;
    llvm::Function *shared_memory_sequential_store;
    llvm::Function *shared_memory_sequential_load;
    llvm::Function *shared_memory_pointer_store;
    llvm::Function *allocate_shared_value;
    llvm::Function *shared_value_store;
    llvm::Function *shared_value_load;
    llvm::Function *shared_value_synchronize;
    llvm::Function *modify_parallel_for_bounds_4;
    llvm::Function *modify_parallel_for_bounds_8;
    llvm::Function *critical_section_init;
    llvm::Function *critical_section_enter;
    llvm::Function *critical_section_leave;
    llvm::Function *critical_section_finalize;
    llvm::Function *reduce_local_vars;

    // netCDF library functions
    llvm::Function *io_open;
    llvm::Function *io_open_par;
    llvm::Function *io_create_par;
    llvm::Function *io_var_par_access;
    llvm::Function *io_inq_varid;
    llvm::Function *io_get_var_int;
    llvm::Function *io_get_vara_int;
    // llvm::Function *io_get_vara_int2;
    // llvm::Function *io_get_vara_int1a;
    // llvm::Function *io_get_vara_int1b;
    llvm::Function *io_close;
    llvm::Function *io_put_vara_int;
    llvm::Function *io_def_var;
};

/**
 * The RuntimeHandler class is used to set up usage of the cato runtime library
 * with the transformed program.
 * It sets up and cleans up MPI and declares the external rtlib functions that
 * can be called by the pass
 **/
class RuntimeHandler
{
  private:
    // The Module the pass is working on
    llvm::Module *_M;

    // Pointer to the block that contains cato setup code
    llvm::BasicBlock *_entry_block;

    // Pointer to the block that contains cato clean up code
    llvm::BasicBlock *_finalize_block;

    // The rtlib module which is loaded by this class to declare
    // the external functions used by the pass
    std::unique_ptr<llvm::Module> _rtlib_module;

    /**
     * Load the rtlib.bc file in the build directory
     **/
    bool load_rtlibs();

    /**
     * Searches for a function with the given name in rtlib and
     * makes it usable with the given llvm::Function* so it can
     * be inserted with an IRBuilder
     **/
    void match_function(llvm::Function **, llvm::StringRef name);

    /**
     * Finds all matching functions from rtlib for the llvm::Constant Pointers
     * in the external_functions struct
     **/
    bool load_external_functions();

  public:
    // llvm::Function Pointers for all external functions that can be used in the pass
    external_functions functions;

    /**
     * Sets up the RuntimeHandler
     * Loads the rtlib and initializes the external_functions struct
     **/
    RuntimeHandler(llvm::Module &M);

    ~RuntimeHandler(){};

    /**
     * Adds the cato initialize and finalize code to the given function
     * if the func parameter is left empty the code will be inserted into
     * the main function of the Module
     **/
    bool insert_cato_init_and_fin(llvm::Function *func = nullptr, bool logging = false);

    void adjust_netcdf_regions();

    /**
     * Replace OpenMP function calls in the Code
     *
     * Currently supported functions:
     *  - omp_get_thread_num
     *  - omp_get_num_threads
     *
     *  TODO add more omp functions
     **/
    void replace_omp_functions();

    llvm::BasicBlock *get_entry_block();

    llvm::BasicBlock *get_finalize_block();
};

#endif
