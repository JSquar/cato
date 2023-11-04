/*
 * File: MemoryAbstractionDefault.cpp
 * -----
 *
 * -----
 * Last Modified: Sat Nov 04 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 */

#include "MemoryAbstractionDefault.h"

#include <cstring>
#include <iostream>
#include <stdio.h>
#include <algorithm>

#include "../debug.h"
#include "CatoRuntimeLogger.h"
#include "CacheHandler.h"
#include "Readahead.h"

MemoryAbstractionDefault::MemoryAbstractionDefault(long size, MPI_Datatype type,
                                                   int dimensions)
    : MemoryAbstraction(size, type, dimensions), _readahead_dt{MPI_DATATYPE_NULL}
{
    if (dimensions == 1)
    {
        create_1d_array(size, type, dimensions);
    }
    else
    {
        _base_ptr = malloc(size);
        Debug(std::cout << dimensions << "D-Array of size: " << size
                        << " allocated at address: " << _base_ptr << "\n";);

        if (auto *logger = CatoRuntimeLogger::get_logger())
        {
            std::string message = std::string("Created ") + std::to_string(dimensions) + "D MemoryAbstractionDefault:\n" +
                                  "   base ptr: " + std::to_string((long)_base_ptr) + "\n" +
                                  "   byte size: " + std::to_string(size);
            *logger << message;
        }
    }
}

MemoryAbstractionDefault::~MemoryAbstractionDefault()
{
    if (auto *logger = CatoRuntimeLogger::get_logger())
    {
        std::string message = std::string("Freeing MemoryAbstractionDefault in destructor:") +
                              "\n" + "   base ptr: " + std::to_string((long)_base_ptr) + "\n" +
                              "   dimensions: " + std::to_string(_dimensions) + "\n" +
                              "   byte size: " + std::to_string(_size_bytes);
        *logger << message;
    }

    Debug(std::cout << "Freeing MemoryAbstractionDefault at address: " << _base_ptr << "\n");

    if (_dimensions == 1)
    {
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Win_free(&_mpi_window);
    }

    if (_base_ptr != nullptr)
    {
        free(_base_ptr);
        _base_ptr = nullptr;
    }

    if (_readahead_dt != MPI_DATATYPE_NULL)
    {
        MPI_Type_free(&_readahead_dt);
    }
}

void MemoryAbstractionDefault::store(void *base_ptr, void *value_ptr, const std::vector<long> indices)
{
    if (_dimensions == 1)
    {
        if (indices.size() == 1)
        {
            auto rank_and_disp = get_target_rank_and_disp_for_offset(indices[0]);

            if (auto *logger = CatoRuntimeLogger::get_logger())
            {
                std::string message =
                    std::string("Store in 1D MemoryAbstractionDefault:\n") +
                    "   base ptr: " + std::to_string((long)_base_ptr) + "\n" +
                    "   local element count: " + std::to_string(_local_num_elements) + "\n" +
                    "   global element count: " + std::to_string(_global_num_elements) + "\n" +
                    "   store index: " + std::to_string(rank_and_disp.second);
                *logger << message;
            }

            //MPI_Win_lock(MPI_LOCK_SHARED, rank_and_disp.first, 0, _mpi_window);
            MPI_Win_lock(MPI_LOCK_SHARED, rank_and_disp.first, MPI_MODE_NOCHECK, _mpi_window);
            MPI_Put(value_ptr, 1, _type, rank_and_disp.first, rank_and_disp.second, 1, _type,
                    _mpi_window);
            MPI_Win_unlock(rank_and_disp.first, _mpi_window);
        }
        else
        {
            std::cerr << "MemoryAbstractionDefault does not support > 1D arrays yet\n";
        }
    }
}

void MemoryAbstractionDefault::load(void *base_ptr, void *dest_ptr, const std::vector<long> indices)
{
    if (_dimensions == 1)
    {
        if (indices.size() == 1)
        {
            auto rank_and_disp = get_target_rank_and_disp_for_offset(indices[0]);

            if (auto *logger = CatoRuntimeLogger::get_logger())
            {
                std::string message =
                    std::string("Load in 1D MemoryAbstractionDefault:\n") +
                    "   base ptr: " + std::to_string((long)_base_ptr) + "\n" +
                    "   local element count: " + std::to_string(_local_num_elements) + "\n" +
                    "   global element count: " + std::to_string(_global_num_elements) + "\n" +
                    "   load index: " + std::to_string(rank_and_disp.second);
                *logger << message;
            }

            //MPI_Win_lock(MPI_LOCK_SHARED, rank_and_disp.first, 0, _mpi_window);
            MPI_Win_lock(MPI_LOCK_SHARED, rank_and_disp.first, MPI_MODE_NOCHECK, _mpi_window);
            MPI_Get(dest_ptr, 1, _type, rank_and_disp.first, rank_and_disp.second, 1, _type,
                    _mpi_window);
            MPI_Win_unlock(rank_and_disp.first, _mpi_window);
        }
        else
        {
            std::cerr << "MemoryAbstractionDefault does not support > 1D arrays yet\n";
        }
    }
}

void MemoryAbstractionDefault::sequential_store(void *base_ptr, void *value_ptr,
                                                const std::vector<long> indices)
{
    if (_dimensions == 1)
    {
        if (indices.size() == 1)
        {
            auto rank_and_disp = get_target_rank_and_disp_for_offset(indices[0]);

            if (auto *logger = CatoRuntimeLogger::get_logger())
            {
                std::string message =
                    std::string("Sequential store in 1D MemoryAbstractionDefault:\n") +
                    "   base ptr: " + std::to_string((long)_base_ptr) + "\n" +
                    "   local element count: " + std::to_string(_local_num_elements) + "\n" +
                    "   global element count: " + std::to_string(_global_num_elements) + "\n" +
                    "   store index: " + std::to_string(rank_and_disp.second);
                *logger << message;
            }

            MPI_Win_fence(MPI_MODE_NOPRECEDE, _mpi_window);
            if (_mpi_rank == rank_and_disp.first)
            {
                MPI_Put(value_ptr, 1, _type, rank_and_disp.first, rank_and_disp.second, 1,
                        _type, _mpi_window);
            }
            MPI_Win_fence(MPI_MODE_NOSUCCEED, _mpi_window);
        }
        else
        {
            std::cerr << "MemoryAbstractionDefault does not support > 1D arrays yet\n";
        }
    }
}

void MemoryAbstractionDefault::sequential_load(void *base_ptr, void *dest_ptr,
                                               const std::vector<long> indices)
{
    if (_dimensions == 1)
    {
        if (indices.size() == 1)
        {
            auto rank_and_disp = get_target_rank_and_disp_for_offset(indices[0]);

            if (auto *logger = CatoRuntimeLogger::get_logger())
            {
                std::string message =
                    std::string("Sequential load in 1D MemoryAbstractionDefault:\n") +
                    "   base ptr: " + std::to_string((long)_base_ptr) + "\n" +
                    "   local element count: " + std::to_string(_local_num_elements) + "\n" +
                    "   global element count: " + std::to_string(_global_num_elements) + "\n" +
                    "   load index: " + std::to_string(rank_and_disp.second);
                *logger << message;
            }

            MPI_Win_fence(MPI_MODE_NOPRECEDE | MPI_MODE_NOPUT, _mpi_window);
            MPI_Get(dest_ptr, 1, _type, rank_and_disp.first, rank_and_disp.second, 1, _type,
                    _mpi_window);
            MPI_Win_fence(MPI_MODE_NOSUCCEED, _mpi_window);
        }
        else
        {
            std::cerr << "MemoryAbstractionDefault does not support > 1D arrays yet\n";
        }
    }
}

bool MemoryAbstractionDefault::is_data_local(const std::vector<long>& indices)
{
    auto rank_and_disp = get_target_rank_and_disp_for_offset(indices[0]);
    return rank_and_disp.first == _mpi_rank;
}

void* MemoryAbstractionDefault::get_address_of_local_element(const std::vector<long>& indices)
{
    auto rank_and_disp = get_target_rank_and_disp_for_offset(indices[0]);
    if (rank_and_disp.first != _mpi_rank)
    {
        return nullptr;
    }
    void* target_address = static_cast<char*>(_base_ptr) + rank_and_disp.second * _type_size;
    return target_address;
}

std::pair<int, long> MemoryAbstractionDefault::get_target_rank_and_disp_for_offset(long offset)
{
    if (_dimensions == 1)
    {
        std::pair<int, long> ret;
        for (int i = 0; i < _mpi_size; i++)
        {
            if (offset >= _array_ranges[i].first && offset <= _array_ranges[i].second)
            {
                long target_disp = offset - _array_ranges[i].first;
                ret = {i, target_disp};
                break;
            }
        }
        return ret;
    }
    else
    {
        return {0, 0};
    }
}

void MemoryAbstractionDefault::create_1d_array(long size, MPI_Datatype type, int dimensions)
{
    MPI_Comm_rank(MPI_COMM_WORLD, &_mpi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &_mpi_size);

    _global_num_elements = size / _type_size;

    int div = _global_num_elements / _mpi_size;
    int rest = _global_num_elements % _mpi_size;

    if (_mpi_size > _global_num_elements)
    {
        // TODO Handle this case
        std::cerr << "Error: There are more MPI processes ("<< _mpi_size << ")  than array elements (" << _global_num_elements << ")\n";
    }

    _array_ranges.reserve(_mpi_size);
    for (int rank = 0; rank < _mpi_size; rank++)
    {
        long local_num_elements, local_from, local_to;

        if (rank < rest)
        {
            local_num_elements = div + 1;
            local_from = rank * local_num_elements;
            local_to = local_from + local_num_elements - 1;
        }
        else
        {
            local_num_elements = div;
            local_from = rank * local_num_elements + rest;
            local_to = local_from + local_num_elements - 1;
        }
        _array_ranges[rank] = {local_from, local_to};

        if (rank == _mpi_rank)
        {
            _local_num_elements = local_num_elements;
            _base_ptr = malloc(local_num_elements * _type_size);
            Debug(std::cout << "MemoryAbstractionDefault: rank " << _mpi_rank << " allocated "
                            << local_num_elements * _type_size << " bytes at " << _base_ptr << "\n");
        }
    }

    MPI_Win_create(_base_ptr, _local_num_elements * _type_size, _type_size, MPI_INFO_NULL,
                   MPI_COMM_WORLD, &_mpi_window);

    if (auto *logger = CatoRuntimeLogger::get_logger())
    {
        std::string message =
            std::string("Created 1D MemoryAbstractionDefault:\n") +
            "   base ptr: " + std::to_string((long)_base_ptr) + "\n" +
            "   local element count: " + std::to_string(_local_num_elements) + "\n" +
            "   global element count: " + std::to_string(_global_num_elements) + "\n" +
            "   type size: " + std::to_string(_type_size) + "\n";
        *logger << message;
    }
}

void MemoryAbstractionDefault::pointer_store(void *source_ptr, long dest_index)
{
    if (auto *logger = CatoRuntimeLogger::get_logger())
    {
        std::string message = std::string("Pointer store in MemoryAbstractionDefault:\n");
        *logger << message;
    }

    ((long **)_base_ptr)[dest_index] = (long *)source_ptr;
    Debug(std::cout << "Saved Pointer into Array abstraction : "
                    << ((long **)_base_ptr)[dest_index] << "\n";);
}
