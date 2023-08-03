/*
 * File: Cacheline.h
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Wed Aug 02 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#ifndef CATO_RTLIB_CACHELINE_H
#define CATO_RTLIB_CACHELINE_H

#include <cstdlib>
#include <iostream>

class Cacheline
{
  private:
    void* _data;
    size_t _element_size;

  public:
    Cacheline(void* data, size_t element_size);
    ~Cacheline();

    Cacheline(const Cacheline& other);
    Cacheline& operator=(const Cacheline& other);

    Cacheline(Cacheline&& other);
    Cacheline& operator=(Cacheline&& other);

    void* get_data() const {return _data;};
    size_t get_size() const {return _element_size;}
};

#endif