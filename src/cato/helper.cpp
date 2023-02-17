#include "helper.h"

#include <mpi.h>

using namespace llvm;

std::vector<User *> get_function_users(Module &M, StringRef name)
{
    std::vector<User *> func_users;
    if (Function *func = M.getFunction(name))
    {
        for (auto *user : func->users())
        {
            func_users.push_back(user);
        }
    }
    return func_users;
}

bool is_in_list(llvm::StringRef string, std::vector<std::string> string_list)
{
    for (auto &s : string_list)
    {
        if (string.equals(s))
        {
            return true;
        }
    }
    return false;
}

int get_pointer_depth(Type *type)
{
    int depth = 0;

    while (type->isPointerTy())
    {
        depth++;
        type = type->getPointerElementType();
    }

    return depth;
}

int get_pointer_depth(Value *value)
{
    Type *type = value->getType();

    return get_pointer_depth(type);
}

int get_mpi_datatype(Type *type)
{
    while (type->isPointerTy())
    {
        type = type->getPointerElementType();
    }

    if (type->isIntegerTy(32))
    {
        return MPI_INT;
    }
    else if (type->isIntegerTy(64))
    {
        return MPI_LONG_LONG;
    }
    else if (type->isFloatTy())
    {
        return MPI_FLOAT;
    }
    else if (type->isDoubleTy())
    {
        return MPI_DOUBLE;
    }
    else if (type->isIntegerTy(8))
    {
        return MPI_CHAR;
    }
    // if MPI_BYTE Is returned, caller should use get_size_in_Bytes to determine the size
    // of buffer
    return MPI_BYTE;
}

int get_mpi_datatype(Value *value)
{
    Type *type = nullptr;

    // If it is a shared array
    if (value->getType()->getPointerElementType()->isArrayTy())
    {
        type = value->getType()->getPointerElementType()->getArrayElementType();
    }
    // If it is a pointer with depth > 1
    else if (value->getType()->getPointerElementType()->isPointerTy())
    {
        type = value->getType()->getPointerElementType();
        while (type->isPointerTy())
        {
            type = type->getPointerElementType();
        }
    }
    // If it is a shared single value
    else
    {
        type = value->getType()->getPointerElementType();
    }
    return get_mpi_datatype(type);
}

size_t get_size_in_Byte(llvm::Module &M, llvm::Value *value)
{
    Type *type = nullptr;

    // If it is a shared array
    if (value->getType()->getPointerElementType()->isArrayTy())
    {
        type = value->getType()->getPointerElementType()->getArrayElementType();
    }
    // If it is a pointer with depth > 1
    else if (value->getType()->getPointerElementType()->isPointerTy())
    {
        type = value->getType()->getPointerElementType();
        while (type->isPointerTy())
        {
            type = type->getPointerElementType();
        }
    }
    // If it is a shared single value
    else
    {
        type = value->getType()->getPointerElementType();
    }
    return get_size_in_Byte(M, type);
}

size_t get_size_in_Byte(llvm::Module &M, llvm::Type *type)
{
    DataLayout *TD = new DataLayout(&M);
    return TD->getTypeAllocSize(type);
}
