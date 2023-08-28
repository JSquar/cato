/*
 * File: Readahead.h
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Mon Aug 28 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#ifndef CATO_RTLIB_READHEAD_H
#define CATO_RTLIB_READHEAD_H

#include "CacheHandler.h"
#include "MemoryAbstractionDefault.h"
#include <vector>

void* performReadahead(MemoryAbstractionDefault* mem_abstraction, void* base_ptr, CacheHandler* cache_handler,
                        const std::vector<long>& initial_indices, std::pair<int,long> rank_and_disp, long count);

#endif