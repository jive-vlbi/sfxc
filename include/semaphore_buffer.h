/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#ifndef SEMAPHORE_BUFFER_H
#define SEMAPHORE_BUFFER_H

#include <semaphore.h>
#include <assert.h>
#include <iostream>

#include "buffer.h"

template <class T = Buffer_element<char, 131072> >
class Semaphore_buffer : public Buffer<T> {
public:
  typedef T                                       value_type;
  typedef Buffer<T>                               Base;
  typedef Semaphore_buffer<T>                     Self;

  Semaphore_buffer(int size);
  Semaphore_buffer(int size, const T& element);
  ~Semaphore_buffer();

  // NGHK: add a mutex in produce (multiple producers)
  T &produce();
  void produced(int status);

  // NGHK: add a mutex in consume (multiple consumers)
  T &consume(int &status);
  void consumed();

  bool empty();
  bool full();
private:
  // Two semaphores to avoid overwriting of data
  sem_t empty_sem, full_sem;
  // For debugging purposes:
  bool consuming, producing;
};


///////////////////////////////////////
// IMPLEMENTATION
///////////////////////////////////////


template <class T>
Semaphore_buffer<T>::
Semaphore_buffer(int size)
    : Base(size), consuming(false), producing(false) {
  assert(size > 0);
  if ( sem_init(&empty_sem, 1, 0) == -1 ) {
    std::cout << "Failed to initialise the \"empty\" semaphore" << std::endl;
    exit(1);
  }
  if ( sem_init(&full_sem, 1, size) == -1 ) {
    std::cout << "Failed to initialise the \"full\" semaphore" << std::endl;
    exit(1);
  }
}

template <class T>
Semaphore_buffer<T>::
Semaphore_buffer(int size, const T& element)
    : Base(size, element), consuming(false), producing(false) {
  assert(size > 0);
  if ( sem_init(&empty_sem, 1, 0) == -1 ) {
    std::cout << "Failed to initialise the \"empty\" semaphore" << std::endl;
    exit(1);
  }
  if ( sem_init(&full_sem, 1, size) == -1 ) {
    std::cout << "Failed to initialise the \"full\" semaphore" << std::endl;
    exit(1);
  }
}


template <class T>
Semaphore_buffer<T>::~Semaphore_buffer() {
//  DEBUG_MSG("~Semaphore_buffer()");
}

template <class T>
T &
Semaphore_buffer<T>::produce() {
  assert(!producing);
  producing = true;
  sem_wait(&full_sem);
  return Base::get_prod_elem();
}

template <class T>
void
Semaphore_buffer<T>::produced(int status) {
  assert(producing);
  producing = false;
  Base::succ_prod(status);
  sem_post(&empty_sem);
}

template <class T>
T &
Semaphore_buffer<T>::consume(int &status) {
  assert(!consuming);
  consuming = true;
  sem_wait(&empty_sem);
  return Base::get_cons_elem(status);
}

template <class T>
void
Semaphore_buffer<T>::consumed() {
  assert(consuming);
  consuming = false;
  Base::succ_cons();
  sem_post(&full_sem);
}

template <class T>
bool
Semaphore_buffer<T>::empty() {
  int val;
  sem_getvalue(&empty_sem, &val);
  return (val <= 0);
}

template <class T>
bool
Semaphore_buffer<T>::full() {
  int val;
  sem_getvalue(&full_sem, &val);
  return (val == 0);
}

#endif // SEMAPHORE_BUFFER_H
