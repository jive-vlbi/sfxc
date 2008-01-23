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

#include "utils.h"

/// Use this element for the buffer class if you want to store an array
/// on every position.
template <class T, int N>
class Buffer_element {
public:
  typedef T value_type;
  int size() { return N; }

  T &operator[](int i) {
    assert(i >= 0);
    assert(i < N);
    return _buffer[i];
  }
  T *buffer() {
    return &_buffer[0];
  }
private:
  T _buffer[N];
};

/// Use this class if you want to allocate large number of element
/// static array declaration may fails if the array is too large...
template <class T, int N>
class Buffer_element_large {
public:
  typedef T value_type;
  int size() { return N; }

  Buffer_element_large(){
    //std::cout << "Building array of size" << N << std::endl;
    _buffer = new T[N];
  }

  Buffer_element_large(const Buffer_element_large& src){
    //std::cout << "Building by copy an array of size" << N << std::endl;
    _buffer = new T[N];
    memcpy(_buffer, src._buffer, sizeof(T)*N );
  }

  ~Buffer_element_large(){
    //std::cout << "Deleting array of size" << N << std::endl;
    delete[] _buffer;
  }

  T &operator[](int i) {
    assert(i >= 0);
    assert(i < N);
    return (_buffer)[i];
  }
  T *buffer() {
    return _buffer;
  }
private:
  T* _buffer;
};

/// Use this class if you want to allocate large number of element
/// static array declaration may fails if the array is too large...
template <class T>
class Buffer_element_vector {
public:
  typedef T value_type;
  int size() { return _buffer.size(); }

  Buffer_element_vector() {
  }

  ~Buffer_element_vector(){
//    DEBUG_MSG("Deleting array element of size " << size());
  }

  void resize(size_t size) {
//    DEBUG_MSG("resizing from " << _buffer.size() << " to " << size);
    if (_buffer.size() != size) _buffer.resize(size);
  }


  T &operator[](int i) {
    assert(i >= 0);
    assert(i < (int)_buffer.size());
    return _buffer[i];
  }
  T *buffer() {
    return &_buffer[0];
  }
private:
  std::vector<T> _buffer;
};


/** Generic buffer class.
 * Precondition T is default constructible
 **/
template <class T = Buffer_element_large<char, 131072> >
class Buffer {
public:
  typedef T                                       value_type;
  typedef Buffer<T>                               Self;

  Buffer(int size);
  Buffer(int size, const T &element);
  virtual ~Buffer();

  virtual T &produce() = 0;
  virtual void produced(int status) = 0;

  virtual T &consume(int &status) = 0;
  virtual void consumed() = 0;

  virtual bool empty()= 0;
  virtual bool full()= 0;

protected:
  // Generic functions for a circular buffer:

  T& get_prod_elem() {
    return buffer[rear];
  }
  void succ_prod(int stat) {
    status[rear] = stat;
    rear = (rear+1)%size;
  }

  T& get_cons_elem(int &stat) {
    stat = status[front];
    return buffer[front];
  }
  T& get_cons_elem_prev(int &stat, int prev) {
    assert(prev > 0);
    assert(prev < size);
    stat = status[(front-prev+size)%size];
    return buffer[(front-prev+size)%size];
  }
  void succ_cons() {
    front = (front+1)%size;
  }

protected:
  // Children are allowed to access the buffer size;
  int size;

private:
  // The buffer
  std::vector<T>   buffer;
  std::vector<int> status;
  // indices in the buffer
  int front, rear;
};

template <class T>
Buffer<T>::
Buffer(int size)
  : size(size),
    front(0), rear(0)
{
  if (size <= 0) { DEBUG_MSG(size); }
  assert(size > 0);
  buffer.resize(size);
  status.resize(size);
}

template <class T>
Buffer<T>::
Buffer(int size, const T &element)
  : size(size),
    front(0), rear(0)
{
  if (size <= 0) { DEBUG_MSG(size); }
  assert(size > 0);
  buffer.resize(size, element);
  status.resize(size);
}


template <class T>
Buffer<T>::
~Buffer()
{
}

#endif // BUFFER_H
