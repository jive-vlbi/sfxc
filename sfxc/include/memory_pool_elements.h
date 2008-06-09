/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <assert.h>

// Ugly dependency to fftw3...
// needed for fftw_malloc that return
// optimized alignment.
#include <fftw3.h>

#include "utils.h"

/// Use this class if you want to allocate large number of element
/// static array declaration may fail if the array is too large...
template <class T>
class Memory_pool_vector_element {
public:
  typedef T value_type;
  inline size_t size() const {
    return size_;
  }

  Memory_pool_vector_element() {
    size_ = 0;
    buffer_ = NULL;
  }

  ~Memory_pool_vector_element() {
    // DEBUG_MSG("Deleting array element of size " << size());
    if (buffer_ != NULL) {
      fftw_free(buffer_);
    }
  }

  void resize(size_t size) {
    if ( size != size_ ) {
      if ( buffer_ == NULL ) {
        // fftw_malloc insure that the allocated data
        // is nicely aligned.
        buffer_ = static_cast<T*>( fftw_malloc( sizeof(T)*size ) );
        size_ = size;
      } else {
        T* oldbuffer = buffer_;
        buffer_ = static_cast<T*>( fftw_malloc( sizeof(T)*size ) );
        size_t min_size = std::min(size_,size);
        size_ = size;

        memcpy(buffer_, oldbuffer, sizeof(T)*min_size);

        fftw_free(oldbuffer);
      }
    }
  }


  inline T& operator[](int i) {
    assert(i >= 0);
    assert(i < (int)size_ );
    return buffer_[i];
  }

  inline const T& operator[](int i) const {
    assert(i >= 0);
    assert(i < (int)size_ );
    return buffer_[i];
  }

  inline T* buffer() {
    return buffer_;
  }
private:

  // We cannot use a std::vector because we cannot control that
  // the data are properly aligned to be used with fftw_xx or SSE
  T* buffer_;
  size_t size_;
};

/// Use this class if you want to allocate large number of element
/// static array declaration may fail if the array is too large...
template <class T, int N>
class Memory_pool_fixed_size_element {
public:
  typedef T value_type;
  inline size_t size() const {
    return N;
  }

  Memory_pool_fixed_size_element() {
    // fftw_malloc insure that the allocated data
    // is nicely aligned.
    buffer_ = static_cast<T*>( fftw_malloc(sizeof(T)*N) );
  }

  ~Memory_pool_fixed_size_element() {
    fftw_free(buffer_);
  }

  inline T& operator[](int i) {
    assert(i >= 0);
    assert(i < N );
    return buffer_[i];
  }

  inline const T& operator[](int i) const {
    assert(i >= 0);
    assert(i < N );
    return buffer_[i];
  }

  inline T* buffer() {
    return buffer_;
  }
private:

  // We cannot use a std::vector because we cannot control that
  // the data are properly aligned to be used with fftw_xx or SSE
  T* buffer_;
};

#endif // BUFFER_H
