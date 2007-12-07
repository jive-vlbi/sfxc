#ifndef DEFAULT_ALLOCATOR_H
#define DEFAULT_ALLOCATOR_H

#include "allocator.h"

template<class T>
class Default_allocator : public Allocator<T>
{
    public:
				typedef T   Type;
				typedef T* pType;

				Default_allocator();
        virtual ~Default_allocator();

        virtual pType allocate();
};

template<class T> Default_allocator<T>::Default_allocator(){}
template<class T> Default_allocator<T>::~Default_allocator(){}
template<class T>
typename Default_allocator<T>::pType Default_allocator<T>::allocate(){
	return new T();
}

#endif // DEFAULT_ALLOCATOR_H
