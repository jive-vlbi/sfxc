#ifndef SEMAPHORE_BUFFER_H
#define SEMAPHORE_BUFFER_H

#include <Buffer.h>
#include <semaphore.h>
#include <assert.h>
#include <iostream>

template <class T>
class Semaphore_buffer : public Buffer<T> {
public:
  typedef T                                       value_type;
  typedef Buffer<T>                               Base;
  typedef Semaphore_buffer<T>                     Self;

  Semaphore_buffer(int size);
  ~Semaphore_buffer();

  T &produce();
  void produced(int status);
  
  T &consume(int &status);
  void consumed();

  bool empty();
private:
  // Two semaphores to avoid overwriting of data
  sem_t empty_sem, full_sem;
};


///////////////////////////////////////
// IMPLEMENTATION
///////////////////////////////////////


template <class T>
Semaphore_buffer<T>::
Semaphore_buffer(int size) 
  : Base(size)
{
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
}

template <class T>
T &
Semaphore_buffer<T>::produce() {
  sem_wait(&full_sem);
  return Base::get_prod_elem();
}

template <class T>
void
Semaphore_buffer<T>::produced(int status) {
  Base::succ_prod(status);
  sem_post(&empty_sem);
}
  
template <class T>
T &
Semaphore_buffer<T>::consume(int &status) {
  sem_wait(&empty_sem);
  return Base::get_cons_elem(status);
}

template <class T>
void
Semaphore_buffer<T>::consumed() {
  Base::succ_cons();
  sem_post(&full_sem);
}

template <class T>
bool 
Semaphore_buffer<T>::empty() {
  int val;
  sem_getvalue(&empty_sem, &val);
  return (val > 0);
}

#endif // SEMAPHORE_BUFFER_H
