#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <Buffer.h>
#include <assert.h>
#include <iostream>

template <class T>
class Ring_buffer : public Buffer<T> {
public:
  typedef T                                       value_type;
  typedef Buffer<T>                               Base;
  typedef Ring_buffer<T>                          Self;

  Ring_buffer(int size);
  ~Ring_buffer();

  T &produce();
  void produced(int status);
  
  T &consume(int &status);
  void consumed();


private:
  INT64 empty, full;
};


///////////////////////////////////////
// IMPLEMENTATION
///////////////////////////////////////


template <class T>
Ring_buffer<T>::
Ring_buffer(int size) 
  : Base(size), empty(0), full(0)
{
}

template <class T>
Ring_buffer<T>::~Ring_buffer() {
}

template <class T>
T &
Ring_buffer<T>::produce() {
  if (full >= empty+Base::size) std::cout << "Buffer overflow" << std::endl;
  return Base::get_prod_elem();
}

template <class T>
void
Ring_buffer<T>::produced(int status) {
  full++;
  Base::succ_prod(status);
}
  
template <class T>
T &
Ring_buffer<T>::consume(int &status) {
  if (full <= empty) std::cout << "Buffer underflow" << std::endl;
  return Base::get_cons_elem(status);
}

template <class T>
void
Ring_buffer<T>::consumed() {
  empty++;
  Base::succ_cons();
}

#endif // RING_BUFFER_H
