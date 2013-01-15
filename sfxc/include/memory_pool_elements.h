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

#include "utils.h"
#include <vector>

#include "align_malloc.h"


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
      aligned_free(buffer_);
    }
  }

  Memory_pool_vector_element(const Memory_pool_vector_element<T> &other):buffer_(NULL), size_(0){
    resize(other.size_);
    memcpy(buffer_, other.buffer_, sizeof(T)*size_);
  }

  void resize(size_t size) {
    if ( size != size_ ) {
      if ( buffer_ == NULL ) {
        // aligned_malloc insure that the allocated data
        // is nicely aligned.
        buffer_ = static_cast<T*>( aligned_malloc( sizeof(T)*size ) );
        size_ = size;
      } else {
        T* oldbuffer = buffer_;
        buffer_ = static_cast<T*>( aligned_malloc( sizeof(T)*size ) );
        size_t min_size = std::min(size_,size);
        size_ = size;

        memcpy(buffer_, oldbuffer, sizeof(T)*min_size);

        aligned_free(oldbuffer);
      }
    }
  }


  inline T& operator[](int i) {
    SFXC_ASSERT(i >= 0);
    SFXC_ASSERT(i < (int)size_ );
    return buffer_[i];
  }

  inline const T& operator[](int i) const {
    SFXC_ASSERT(i >= 0);
    SFXC_ASSERT(i < (int)size_ );
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
    // aligned_malloc insure that the allocated data
    // is nicely aligned.
    buffer_ = static_cast<T*>( aligned_malloc(sizeof(T)*N) );
  }

  ~Memory_pool_fixed_size_element() {
    aligned_free(buffer_);
  }

  inline T& operator[](int i) {
    SFXC_ASSERT(i >= 0);
    SFXC_ASSERT(i < N );
    return buffer_[i];
  }

  inline const T& operator[](int i) const {
    SFXC_ASSERT(i >= 0);
    SFXC_ASSERT(i < N );
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
