/*
 * File: WriteCacheLine.cpp
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Thu Aug 10 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#include <algorithm>
#include <cstdlib>
#include <cstring>

#include "WriteCacheLine.h"

WriteCacheLine::WriteCacheLine(MPI_Win win, MPI_Datatype type)
    : _win{win}, _type{type}
{
    int comm_size;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Type_size(type, &_type_size);
    _cached_elements.resize(comm_size);
}

void WriteCacheLine::insert_element(void* data, int target_rank, long displacement)
{
    _cached_elements[target_rank].push_back({displacement, {data, static_cast<size_t>(_type_size)}});
}

void WriteCacheLine::clear_cache()
{
    for (size_t i = 0; i < _cached_elements.size(); i++)
    {
        auto& rank_vector = _cached_elements[i];
        if (rank_vector.empty())
        {
            continue;
        }

        //Sorts all cache elements by displacements in ascending order. Then the content of the cache elements is
        //concatenated and the displacements are extracted. Based on these displacements, a new MPI datatype is
        //created and used to transfer the data in one operation.
        std::sort(rank_vector.begin(), rank_vector.end(),
                [](std::pair<long,CacheElement> left, std::pair<long,CacheElement> right) {return left.first < right.first;});

        std::vector<int> displacements;
        void* buffer = std::malloc(_type_size * rank_vector.size());

        for (size_t j = 0; j < rank_vector.size(); j++)
        {
            auto& elem = rank_vector[j];
            displacements.push_back(static_cast<int>(elem.first));
            void* target_address = static_cast<char*>(buffer) + j * _type_size;
            std::memcpy(target_address, elem.second.get_data(), _type_size);
        }

        MPI_Datatype dt;
        std::vector<int> block_lengths (displacements.size(), 1);
        MPI_Type_indexed(displacements.size(), block_lengths.data(), displacements.data(), _type, &dt);
        MPI_Type_commit(&dt);

        MPI_Win_lock(MPI_LOCK_EXCLUSIVE, i, 0, _win);
        MPI_Put(buffer, displacements.size(), _type, i, 0, 1, dt, _win);
        MPI_Win_unlock(i, _win);

        MPI_Type_free(&dt);
        std::free(buffer);
        rank_vector.clear();
    }
}