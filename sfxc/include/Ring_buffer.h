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

  bool empty() { return ptr_empty==ptr_full; }
  bool full() { return (ptr_full >= ptr_empty+Base::size); }

private:
  INT64 ptr_empty, ptr_full;
};


///////////////////////////////////////
// IMPLEMENTATION
///////////////////////////////////////


template <class T>
Ring_buffer<T>::
Ring_buffer(int size) 
  : Base(size), ptr_empty(0), ptr_full(0)
{
}

template <class T>
Ring_buffer<T>::~Ring_buffer() {
}

template <class T>
T &
Ring_buffer<T>::produce() {
  if (ptr_full >= ptr_empty+Base::size) 
    std::cout << "Buffer overflow" << std::endl;
  return Base::get_prod_elem();
}

template <class T>
void
Ring_buffer<T>::produced(int status) {
  ptr_full++;
  Base::succ_prod(status);
}
  
template <class T>
T &
Ring_buffer<T>::consume(int &status) {
  if (ptr_full <= ptr_empty) std::cout << "Buffer underflow" << std::endl;
  return Base::get_cons_elem(status);
}

template <class T>
void
Ring_buffer<T>::consumed() {
  ptr_empty++;
  Base::succ_cons();
}

#endif // RING_BUFFER_H
