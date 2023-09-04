/*
 * File: Readahead.h
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Mon Sep 04 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#ifndef CATO_RTLIB_READHEAD_H
#define CATO_RTLIB_READHEAD_H

#include "CacheHandler.h"
#include "MemoryAbstractionDefault.h"
#include <vector>

void* perform_readahead(MemoryAbstractionDefault* mem_abstraction, void* base_ptr, CacheHandler* const cache_handler,
                        const std::vector<long>& initial_indices, const std::vector<long>& indices);

#endif