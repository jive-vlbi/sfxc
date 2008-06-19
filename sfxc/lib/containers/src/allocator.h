#ifndef ALLOCATOR_H_INCLUDED

#define ALLOCATOR_H_INCLUDED


#include <tr1/memory>


template<class T>

class Allocator {

public:
  typedef std::tr1::shared_ptr< Allocator<T> > SelfPtr;


  virtual ~Allocator() {};

  virtual T* allocate() = 0;

};




#endif // ALLOCATOR_H_INCLUDED

