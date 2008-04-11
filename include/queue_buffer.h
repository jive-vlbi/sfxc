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

#include "buffer.h"

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
  bool full() {
    return false;
  }

private:
  // One semaphores to avoid reading from an empty queue:
  sem_t empty_sem;

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
    : Base(1) {
  if ( sem_init(&empty_sem, 1, 0) == -1 ) {
    std::cout << "Failed to initialise the \"empty_sem\" semaphore" << std::endl;
    exit(1);
  }
}

template <class T>
Queue_buffer<T>::~Queue_buffer() {
}

template <class T>
T &
Queue_buffer<T>::produce() {
  buffer.push(new T());
  return *buffer.back();
}

template <class T>
void
Queue_buffer<T>::produced(int nelem) {
  status.push(nelem);
  sem_post(&empty_sem);
}

template <class T>
T &
Queue_buffer<T>::consume(int &nelem) {
  sem_wait(&empty_sem);
  nelem = status.front();
  return *buffer.front();
}

template <class T>
void
Queue_buffer<T>::consumed() {
  status.pop();
  delete buffer.front();
  buffer.pop();
}

template <class T>
bool
Queue_buffer<T>::empty() {
  int val;
  sem_getvalue(&empty_sem, &val);
  return (val == 0);
}

#endif // QUEUE_BUFFER_H
