#ifndef DEFAULT_ALLOCATOR_H
#define DEFAULT_ALLOCATOR_H

#include "allocator.h"

template<class T>
class Default_allocator : public Allocator<T>
{
    public:
        typedef boost::shared_ptr<Default_allocator<T> > SelfPtr;
        
				typedef T   Type;
				typedef T* pType;

        virtual ~Default_allocator();

        virtual pType allocate();
                    
        static SelfPtr create(){ 
          return SelfPtr( new Default_allocator() ); 
        }
        
     private:
				Default_allocator();
};

template<class T> Default_allocator<T>::Default_allocator(){}

template<class T> Default_allocator<T>::~Default_allocator(){}

template<class T>
typename Default_allocator<T>::pType Default_allocator<T>::allocate(){
	return new T();
}

#endif // DEFAULT_ALLOCATOR_H
