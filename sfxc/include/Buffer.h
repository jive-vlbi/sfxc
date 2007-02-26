#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <assert.h>

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

/** Generic buffer class.
 * Precondition T is default constructible
 **/
template <class T>
class Buffer {
public:
  typedef T                                       value_type;
  typedef Buffer<T>                               Self;

  Buffer(int size);
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
  void succ_cons() {
    front = (front+1)%size;
  }

  // Children are allowed to access the buffer size;
  int size;

private:
  // The buffer
  std::vector<T> buffer;
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
  assert(size > 0);
  buffer.resize(size);
  status.resize(size);
}


template <class T>
Buffer<T>::
~Buffer()
{
}

#endif // BUFFER_H
