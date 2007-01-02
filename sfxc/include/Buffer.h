#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <assert.h>

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

protected:
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

  // Children are allowed to the the buffer size;
  int size;

private:
  // The buffer
  T *buffer;
  int *status;
  // indices in the buffer
  int front, rear;
};

template <class T>
Buffer<T>::
Buffer(int size) 
  : size(size), 
    buffer(new T[size]), status(new int[size]), 
    front(0), rear(0)
{
  assert(size > 0);
}


template <class T>
Buffer<T>::
~Buffer()
{
  delete[](buffer);
}

#endif // BUFFER_H
