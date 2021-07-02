#ifndef CATO_HELPER_H
#define CATO_HELPER_H

#include <llvm/ADT/StringRef.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

#include <vector>

/**
 * Returns a vector of all Users of the named function
 * Can be used to find all instances where the named function is called in the
 * IR Module M
 **/
std::vector<llvm::User *> get_function_users(llvm::Module &, llvm::StringRef);

/**
 * Returns true if the given StringRef is a member of the given string list
 **/
bool is_in_list(llvm::StringRef string, std::vector<std::string> string_list);

/**
 * Searches the Function for all uses of instructions of type T and returns them in a
 * vector
 **/
template <class T> std::vector<T *> get_instruction_in_function(llvm::Function *func)
{
    std::vector<T *> instructions;
    for (auto &B : *func)
    {
        for (auto &I : B)
        {
            if (auto *inst = llvm::dyn_cast<T>(&I))
            {
                instructions.push_back(inst);
            }
        }
    }
    return instructions;
}

/**
 * Searches the whole Module for all instructions of Type T and returns them in a
 * vector
 **/
template <class T> std::vector<T *> get_instruction_in_module(llvm::Module &M)
{
    std::vector<T *> instructions;
    for (auto &F : M)
    {
        for (auto &B : F)
        {
            for (auto &I : B)
            {
                if (auto *instruction = llvm::dyn_cast<T>(&I))
                {
                    instructions.push_back(instruction);
                }
            }
        }
    }
    return instructions;
}

/**
 * Returns the length of the pointer chain of a given value
 * For example the base pointer to a 2D array (int** arr) would return 2
 **/
int get_pointer_depth(llvm::Type *);
int get_pointer_depth(llvm::Value *);

/**
 * Returns the corresponding MPI_Datatype for a llvm Type
 **/
int get_mpi_datatype(llvm::Type *);
int get_mpi_datatype(llvm::Value *);

/**
 * Returns the size of the given Type in bytes
 **/
size_t get_size_in_Byte(llvm::Module &, llvm::Value *);
size_t get_size_in_Byte(llvm::Module &, llvm::Type *);
#endif
