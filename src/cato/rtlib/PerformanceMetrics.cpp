/*
 * File: PerformanceMetrics.cpp
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Mon Aug 21 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */


#include "PerformanceMetrics.h"

#include <mpi.h>
#include <sys/resource.h>
#include <iostream>

namespace
{
    #if CAPTURE_MEMORY_METRICS == 1
    long mem_peak_at_start_in_kb;
    long mem_peak_at_end_in_kb;
    #endif

    #if CAPTURE_CACHE_METRICS == 1
    unsigned long cache_hits_index;
    unsigned long cache_hits_read;

    unsigned long cache_misses_index;
    unsigned long cache_misses_read;

    unsigned long aggregated_write_calls;
    unsigned long executed_write_calls;
    #endif
}

void peak_memory_usage_at_start()
{
    #if CAPTURE_MEMORY_METRICS == 1
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    mem_peak_at_start_in_kb = usage.ru_maxrss;
    #endif
}

void peak_memory_usage_at_end()
{
    #if CAPTURE_MEMORY_METRICS == 1
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    mem_peak_at_end_in_kb = usage.ru_maxrss;
    #endif
}

void cache_hit(enum CACHETYPE cachetype)
{
    #if CAPTURE_CACHE_METRICS == 1
    if (cachetype == CACHETYPE::INDEX)
    cache_hits_index++;
    else if (cachetype == CACHETYPE::READ)
    cache_hits_read++;
    #endif
}

void cache_miss(enum CACHETYPE cachetype)
{
    #if CAPTURE_CACHE_METRICS == 1
    if (cachetype == CACHETYPE::INDEX)
    cache_misses_index++;
    else if (cachetype == CACHETYPE::READ)
    cache_misses_read++;
    #endif
}

void report_aggregated_elements(unsigned long count)
{
    #if CAPTURE_CACHE_METRICS == 1
    executed_write_calls++;
    aggregated_write_calls+=count;
    #endif
}

void print_metrics()
{
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    #if CAPTURE_MEMORY_METRICS == 1
    long peak_sum_start;
    MPI_Reduce(&mem_peak_at_start_in_kb, &peak_sum_start, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    long peak_sum_end;
    MPI_Reduce(&mem_peak_at_end_in_kb, &peak_sum_end, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        std::cout << "PEAK MEMORY USAGE SUMMED: " << peak_sum_end << " KB\n";
        std::cout << "AVG MEMORY USAGE PER PROC: " << peak_sum_end/size << " KB\n";
        std::cout << "PEAK MEMORY USAGE SUMMED (AFTER SUBTRACTING START PEAK): " << peak_sum_end - peak_sum_start << " KB\n";
        std::cout << "AVG MEMORY USAGE PER PROC (AFTER SUBTRACTING START PEAK): " << (peak_sum_end - peak_sum_start)/size << " KB\n";
    }
    #endif

    #if CAPTURE_CACHE_METRICS == 1
    unsigned long cache_hits_index_sum;
    MPI_Reduce(&cache_hits_index, &cache_hits_index_sum, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    unsigned long cache_misses_index_sum;
    MPI_Reduce(&cache_misses_index, &cache_misses_index_sum, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    unsigned long cache_hits_read_sum;
    MPI_Reduce(&cache_hits_read, &cache_hits_read_sum, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    unsigned long cache_misses_read_sum;
    MPI_Reduce(&cache_misses_read, &cache_misses_read_sum, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    unsigned long aggregated_write_calls_sum;
    MPI_Reduce(&aggregated_write_calls, &aggregated_write_calls_sum, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    unsigned long executed_write_calls_sum;
    MPI_Reduce(&executed_write_calls, &executed_write_calls_sum, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        std::cout << "INDEX CACHE HITS SUMMED: " << cache_hits_index_sum << "\n";
        std::cout << "INDEX CACHE MISSES SUMMED: " << cache_misses_index_sum << "\n";
        std::cout << "READ CACHE HITS SUMMED: " << cache_hits_read_sum << "\n";
        std::cout << "READ CACHE MISSES SUMMED: " << cache_misses_read_sum << "\n";
        std::cout << "AGGREGATED WRITE CALLS SUMMED: " << aggregated_write_calls_sum << "\n";
        std::cout << "TOTAL WRITE CALLS EXECUTED INSTEAD: " << executed_write_calls_sum << "\n";
    }
    #endif
}