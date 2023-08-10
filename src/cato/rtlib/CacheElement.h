/*
 * File: CacheElement.h
 * Author: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Last Modified: Thu Aug 10 2023
 * Modified By: Niclas Schroeter (niclas.schroeter@uni-hamburg.de)
 * -----
 * Copyright (c) 2023 Niclas Schroeter
 */

#ifndef CATO_RTLIB_CACHEELEMENT_H
#define CATO_RTLIB_CACHEELEMENT_H

#include <cstdlib>
#include <iostream>


/**
 * A resource container for the data in the caches. The content that is
 * to be cached is copied into this element, not moved.
 **/
class CacheElement
{
  private:
    void* _data;
    size_t _element_size;

  public:
    CacheElement(void* data, size_t element_size);
    ~CacheElement();

    CacheElement(const CacheElement& other);
    CacheElement& operator=(const CacheElement& other);

    CacheElement(CacheElement&& other);
    CacheElement& operator=(CacheElement&& other);

    void* get_data() const {return _data;};
    size_t get_size() const {return _element_size;}
};

#endif