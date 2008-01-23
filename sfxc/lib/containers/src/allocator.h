#ifndef ALLOCATOR_H_INCLUDED
#define ALLOCATOR_H_INCLUDED

template<class T>
class Allocator
{
    public:
        virtual ~Allocator(){};
        virtual T* allocate() = 0;
};

#endif // ALLOCATOR_H_INCLUDED
