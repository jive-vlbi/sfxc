/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef QUEUE_BUFFER_H
#define QUEUE_BUFFER_H

#include <semaphore.h>
#include <assert.h>
#include <iostream>
#include <queue>

#include "Buffer.h"
#include "tasklet/mutex.h"

template <class T>
class Queue_buffer : public Buffer<T> {
public:
  typedef T                                       value_type;
  typedef Buffer<T>                               Base;
  typedef Queue_buffer<T>                     Self;

  Queue_buffer();
  ~Queue_buffer();

  T &produce();
  void produced(int status);
  
  T &consume(int &status);
  void consumed();

  bool empty();
  bool full() { return false; }

private:
  Condition empty_cond;
  
  
  // Override the buffer and status from base:
  std::queue<T *> buffer;
  std::queue<int> status;
};


///////////////////////////////////////
// IMPLEMENTATION
///////////////////////////////////////


template <class T>
Queue_buffer<T>::
Queue_buffer() 
  : Base(1)
{
}

template <class T>
Queue_buffer<T>::~Queue_buffer() {
}

template <class T>
T &
Queue_buffer<T>::produce() {
  RAIIMutex block_mutex(empty_cond);
  buffer.push(new T());
  return *buffer.back();
}

template <class T>
void
Queue_buffer<T>::produced(int nelem) {
  RAIIMutex block_mutex(empty_cond);

  status.push(nelem);
  empty_cond.signal();

  // destructor of block_mutex releases the mutex
}
  
template <class T>
T &
Queue_buffer<T>::consume(int &nelem) {
  RAIIMutex block_mutex(empty_cond);
  if (buffer.empty()) empty_cond.wait();

  nelem = status.front();
  return *buffer.front();

  // destructor of block_mutex releases the mutex
}

template <class T>
void
Queue_buffer<T>::consumed() {
  RAIIMutex block_mutex(empty_cond);
  status.pop();
  delete buffer.front();
  buffer.pop();
}

template <class T>
bool
Queue_buffer<T>::empty() {
  RAIIMutex block_mutex(empty_cond);
  return buffer.empty();
}

#endif // QUEUE_BUFFER_H
