/*
 * File: PerformanceMetrics.h
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Sun Sep 03 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#ifndef CATO_RTLIB_PERFORMANCE_METRICS_H
#define CATO_RTLIB_PERFORMANCE_METRICS_H

#define CAPTURE_MEMORY_METRICS 0

#define CAPTURE_CACHE_METRICS 0

enum class CACHETYPE {INDEX,READ};

void peak_memory_usage_at_start();

void peak_memory_usage_at_end();

void cache_hit(enum CACHETYPE cachetype);

void cache_miss(enum CACHETYPE cachetype);

void report_aggregated_write_elements(unsigned long count);

void report_elements_read_ahead(unsigned long count);

void print_metrics();

#endif