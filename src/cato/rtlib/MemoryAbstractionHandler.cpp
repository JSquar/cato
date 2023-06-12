#include "MemoryAbstractionHandler.h"

#include <stdlib.h>
#include <utility>
#include <functional>

#include <iostream>

#include "MemoryAbstractionDefault.h"
#include "MemoryAbstractionSingleValueDefault.h"

#include "../debug.h"

MemoryAbstractionHandler::MemoryAbstractionHandler(int rank, int size)
{
    _mpi_rank = rank;
    _mpi_size = size;
}

void *MemoryAbstractionHandler::create_memory(long size, MPI_Datatype type, int dimensions)
{
    // Base address of the allocated memory
    void *memory = nullptr;

    auto memory_abstraction =
        std::make_unique<MemoryAbstractionDefault>(size, type, dimensions);
    memory = memory_abstraction->get_base_ptr();

    // Transfer ownership of the unique_ptr to the _memory_abstractions datastructure
    _memory_abstractions.insert(
        std::make_pair((long)memory, std::move(memory_abstraction)));
    Debug(std::cout << "Created a memory abstraction at address: " << memory << "\n";);

    return memory;
}

void MemoryAbstractionHandler::free_memory(void *base_ptr)
{
    // Delete the shared memory object.
    // All related memory has to be freed by MemoryAbstractions destructor.

    if (_memory_abstractions.find((long)base_ptr) != _memory_abstractions.end())
    {
        _memory_abstractions.erase((long)base_ptr);
    }
    else
    {
        Debug(std::cout << "Trying to free an address that does not point to an existing "
                           "MemoryAbstraction. Freeing normally\n";);
        if (base_ptr != nullptr)
        {
            free(base_ptr);
        }
    }
}

MemoryAbstraction* MemoryAbstractionHandler::dereference_pointers(MemoryAbstraction* const memory_abstraction,
                                                                    const std::vector<long> indices)
{
    size_t dimensions = indices.size();
    MemoryAbstraction* current_memory = memory_abstraction;
    long* base_ptr = (long*) memory_abstraction->get_base_ptr();

    for (size_t i = 0; i < dimensions - 2; i++)
    {
        long current_index = indices[i];
        if (_memory_abstractions.find((long)base_ptr[current_index]) != _memory_abstractions.end())
        {
            current_memory = _memory_abstractions[(long)base_ptr[current_index]].get();
            base_ptr = (long*) current_memory->get_base_ptr();
        }
        else
        {
            std::cerr << "Could not find MemoryAbstraction, aborting\n";
            exit(1);
        }
    }

    return current_memory;
}

std::pair<std::vector<long>, MemoryAbstraction*>
MemoryAbstractionHandler::get_elements_per_dimension(MemoryAbstraction* memory_abstraction, size_t dimensions)
{
    std::vector<long> num_elements_in_dimension {};
    num_elements_in_dimension.push_back(memory_abstraction->get_size_bytes() / sizeof(long*));

    //The content of memory_abstraction after the loop will be the one containing all data points
    for (size_t i = 0; i < dimensions - 1; i++)
    {
        long* base_ptr = (long*) memory_abstraction->get_base_ptr();
        if (_memory_abstractions.find((long)base_ptr[0]) != _memory_abstractions.end())
        {
            memory_abstraction = _memory_abstractions[(long)base_ptr[0]].get();
            long byte_size = memory_abstraction->get_size_bytes();
            if (i != dimensions - 2)
            {
                num_elements_in_dimension.push_back(byte_size / sizeof(long*));
            }
            else
            {
                int type_size;
                MPI_Type_size(memory_abstraction->get_type(), &type_size);
                num_elements_in_dimension.push_back(byte_size / type_size);
            }
        }
        else
        {
            std::cerr << "Could not find MemoryAbstraction, aborting\n";
            exit(1);
        }
    }

    return std::make_pair(num_elements_in_dimension, memory_abstraction);
}

long MemoryAbstractionHandler::calculate_new_index(const std::vector<long> indices,
                                                    const std::vector<long> num_elements_in_dimension)
{
    size_t dimensions = indices.size();
    long total_number_of_datapoints = num_elements_in_dimension[dimensions - 1];
    std::vector<long> elements_per_increment_in_dim (dimensions, 1);
    long product_of_element_counts = 1;

    for (long i = dimensions - 2; i >= 0; i--)
    {
        product_of_element_counts *= num_elements_in_dimension[i];
        elements_per_increment_in_dim[i] = elements_per_increment_in_dim[i+1] * num_elements_in_dimension[i];
    }
    long num_elements_last_dimension = total_number_of_datapoints / product_of_element_counts;

    std::transform(elements_per_increment_in_dim.begin(), elements_per_increment_in_dim.end(),
        elements_per_increment_in_dim.begin(),
        std::bind(std::multiplies<long>(), std::placeholders::_1, num_elements_last_dimension));

    long new_index = indices[dimensions - 1];
    for (size_t i = 0; i < dimensions - 1; i++)
    {
        new_index += indices[i] * elements_per_increment_in_dim[i+1];
    }

    return new_index;
}

/**
 * There are different ways to realize nD-arrays in C, as reflected in the test cases for multidimensional arrays,
 * suffixed with v1 and v2 respectively.
 * For v1-style usage, we can just follow our pointers all the way to the lowest dimension, where the actual data is stored and
 * call the store instruction on that memory with the final index.
 * For v2-style use, the second-to-last dimension will not point to the beginning of any memory area, but rather somewhere in the
 * middle. So we essentially have to calculate the index on the lowest dimension, which contains all data points, from the
 * sizes of the higher-level dimensions.
 **/
std::pair<MemoryAbstraction*, long> MemoryAbstractionHandler::get_target_of_operation(void* base_ptr, std::vector<long> indices)
{
    MemoryAbstraction *memory_abstraction = nullptr;

    if (_memory_abstractions.find((long)base_ptr) != _memory_abstractions.end())
    {
        memory_abstraction = _memory_abstractions[(long)(base_ptr)].get();
    }
    else
    {
        std::cerr << "Error: Cato Runtime is trying to access an invalid memory section\n";
        std::cerr << "Shutting down\n";
        exit(1);
    }

    if (indices.size() == 1)
    {
        return std::make_pair(memory_abstraction, indices[0]);
    }
    else
    {
        //Traverse to second-to-last dimension
        size_t dimensions = indices.size();
        MemoryAbstraction* current_memory = dereference_pointers(memory_abstraction, indices);
        long* base_ptr = (long*) current_memory->get_base_ptr();

        //Find memory abstraction? -> v1-style usage, call store with the last index on the retrieved memory
        long second_to_last_index = indices[dimensions-2];
        if (_memory_abstractions.find((long)base_ptr[second_to_last_index]) != _memory_abstractions.end())
        {
            current_memory = _memory_abstractions[(long)base_ptr[second_to_last_index]].get();
            return std::make_pair(current_memory, indices[dimensions - 1]);
        }
        //Else: v2-style usage, calculate new index and store on the lowest dimension MemoryAbstraction
        else
        {
            std::vector<long> num_elements_in_dimension;
            std::tie(num_elements_in_dimension, memory_abstraction) = get_elements_per_dimension(memory_abstraction, dimensions);
            long new_index = calculate_new_index(indices, num_elements_in_dimension);
            return std::make_pair(memory_abstraction, new_index);
        }
    }
}

void MemoryAbstractionHandler::store(void *base_ptr, void *value_ptr,
                                     std::vector<long> indices)
{
    MemoryAbstraction* memory_abstraction = nullptr;
    long index = 0;
    std::tie(memory_abstraction, index) = get_target_of_operation(base_ptr, indices);
    memory_abstraction->store(base_ptr, value_ptr, {index});
}

void MemoryAbstractionHandler::load(void *base_ptr, void *dest_ptr, std::vector<long> indices)
{
    MemoryAbstraction* memory_abstraction = nullptr;
    long index = 0;
    std::tie(memory_abstraction, index) = get_target_of_operation(base_ptr, indices);
    memory_abstraction->load(base_ptr, dest_ptr, {index});
}

void MemoryAbstractionHandler::sequential_store(void *base_ptr, void *value_ptr,
                                                std::vector<long> indices)
{
    MemoryAbstraction* memory_abstraction = nullptr;
    long index = 0;
    std::tie(memory_abstraction, index) = get_target_of_operation(base_ptr, indices);
    memory_abstraction->sequential_store(base_ptr, value_ptr, {index});
}

void MemoryAbstractionHandler::sequential_load(void *base_ptr, void *dest_ptr,
                                               std::vector<long> indices)
{
    MemoryAbstraction* memory_abstraction = nullptr;
    long index = 0;
    std::tie(memory_abstraction, index) = get_target_of_operation(base_ptr, indices);
    memory_abstraction->sequential_load(base_ptr, dest_ptr, {index});
}

void MemoryAbstractionHandler::pointer_store(void *dest_ptr, void *source_ptr, long dest_index)
{
    if (dest_index > 0)
    {
        dest_ptr = (void *)((char *)dest_ptr - dest_index * sizeof(long *));
    }

    MemoryAbstraction *memory_abstraction = nullptr;
    MemoryAbstraction *memory_abstraction2 = nullptr;
    if (_memory_abstractions.find((long)dest_ptr) != _memory_abstractions.end() &&
        _memory_abstractions.find((long)source_ptr) != _memory_abstractions.end())
    {
        memory_abstraction = _memory_abstractions[(long)dest_ptr].get();
        memory_abstraction2 = _memory_abstractions[(long)source_ptr].get();
    }

    if (memory_abstraction != nullptr && memory_abstraction2 != nullptr)
    {
        memory_abstraction->pointer_store(source_ptr, dest_index);
    }
    else if (memory_abstraction != nullptr && memory_abstraction2 == nullptr)
    {
        Debug(std::cout << "Pointer store to memory abstraction does not store the base "
                           "pointer of other memory abstraction\n");
    }
    else
    {
        std::cerr << "Error: Pointer store to shared memory could not be done\n";
    }
}

void MemoryAbstractionHandler::allocate_shared_value(void *base_ptr, MPI_Datatype type)
{
    void *memory = nullptr;

    if (base_ptr != nullptr)
    {
        Debug(std::cout << "Creating a new MemoryAbstractionSingleValue\n";);

        auto memory_abstraction =
            std::make_unique<MemoryAbstractionSingleValueDefault>(base_ptr, type);
        memory = memory_abstraction->get_base_ptr();

        // Transfer ownership of the unique_ptr to the map datastructure
        _single_value_abstractions.insert(
            std::make_pair((long)memory, std::move(memory_abstraction)));
    }
    else
    {
        std::cerr << "Error: Trying to create a MemoryAbstractionSingleValue with NULL as "
                     "base_ptr\n";
        exit(1);
    }
}

void MemoryAbstractionHandler::shared_value_store(void *base_ptr, void *value_ptr)
{
    MemoryAbstractionSingleValue *memory_abstraction = nullptr;
    if (_single_value_abstractions.find((long)base_ptr) != _single_value_abstractions.end())
    {
        memory_abstraction = _single_value_abstractions[(long)base_ptr].get();
    }

    if (memory_abstraction != nullptr)
    {
        memory_abstraction->store(base_ptr, value_ptr);
    }
}

void MemoryAbstractionHandler::shared_value_load(void *base_ptr, void *dest_ptr)
{
    MemoryAbstractionSingleValue *memory_abstraction = nullptr;
    if (_single_value_abstractions.find((long)base_ptr) != _single_value_abstractions.end())
    {
        memory_abstraction = _single_value_abstractions[(long)base_ptr].get();
    }

    if (memory_abstraction != nullptr)
    {
        memory_abstraction->load(base_ptr, dest_ptr);
    }
}

void MemoryAbstractionHandler::shared_value_synchronize(void *base_ptr)
{
    MemoryAbstractionSingleValue *memory_abstraction = nullptr;
    if (_single_value_abstractions.find((long)base_ptr) != _single_value_abstractions.end())
    {
        memory_abstraction = _single_value_abstractions[(long)base_ptr].get();
    }

    if (memory_abstraction != nullptr)
    {
        memory_abstraction->synchronize(base_ptr);
    }
}
