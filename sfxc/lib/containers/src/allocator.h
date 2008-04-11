#ifndef ALLOCATOR_H_INCLUDED
#define ALLOCATOR_H_INCLUDED

#include <boost/shared_ptr.hpp>

template<class T>
class Allocator
{
    public:
        typedef boost::shared_ptr< Allocator<T> > SelfPtr;
        
        virtual ~Allocator(){};
        virtual T* allocate() = 0;
};


#endif // ALLOCATOR_H_INCLUDED
