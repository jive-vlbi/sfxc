/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 *
 * This file is part of:
 *   - containers library
 * This file contains:
 *   - Declaration and definition of the pc_buffer
 */
#ifndef PC_BUFFER_H
#define PC_BUFFER_H

#include <queue>
#include <vector>
#include "exception_common.h"

#include "raiimutex.h"
#include "condition.h"

#include "allocator.h"
#include "default_allocator.h"

#ifdef ENABLE_TEST_UNIT
#include "Test_unit.h"
#endif // ENABLE_TEST_UNIT

/************************************
* @class PC_buffer
* @desc A PC_buffer is used to control
* ownership of objects T. An object
* can be allocated from the buffer,
* manipulated and then destroyed
* (queued back) into the buffer.
* allocation and release of the data
* stored in the buffer are with an
* O(1) complexity.
* Futur plan:
*  we should be able to specified
*  which strategy the PC_buffer could
*  employ to deceide when allocate new
*  Buffer_element. Different policy:
*    - fixed size buffer
*    - double the size of buffer
*    - double the size of buffer but with
*      limited increase,
*    - double the size of buffer with limited
*      increase and throwing an exception
*      in case of overflow.
*************************************/
template<class T>
class PC_buffer
{
public:
    /************************************
    * @class Buffer_element
    * @desc This is the object return by
    * the buffer.
    *************************************/
    class Buffer_element
    {
    public:
        /***************************************
        * Typedef to the real object type
        * accessible using  this Buffer_element
        ****************************************/
        typedef T Type;
        typedef Type* pType;

        /***************************************
        * Returns a pointer to the object stores in
        * the buffer element.
        ****************************************/
        pType data();

        /***************************************
        * The buffer_element is release and goes
        * back into the PC_buffer. The data should
        * not be access further. It is user responsability
        * to not use the object return data() after
        * the release call.
        * throw an exception in case of double
        * release.
        ****************************************/
        void release();
    private:
        friend class PC_buffer<T>;
        Buffer_element(pType data);

				void set_owner(PC_buffer<T>* owner);

        // Pointer to the data
        T* m_data;

        // Pointer to the buffer that allocated the data
        PC_buffer<T>* m_owner;
    };

    /***************************************
    	* Typedef to the real object type
    * accessible using  this PC_buffer
    ****************************************/
    typedef Buffer_element Type;
    typedef Buffer_element* pType;

    /*****************************************
    * Construct a PC_buffer object containing
    * numelements that are allocated using the
    * provided allocator.
    *****************************************/
    PC_buffer(unsigned int numelements=10);
    PC_buffer(unsigned int numelements, Allocator<T>* allocator );
    PC_buffer(unsigned int numelements, Allocator<T>& allocator );

    /************************************
    * Return the number of free Buffer_elements
    * in the buffer. The function is
    * thread-safe and non-blocking
    *************************************/
    size_t number_free_element();

    /************************************
    * take a free Buffer_element and pass
    * it to the caller. The function is
    * thread-safe and the call may block
    * if a free element is not available.
    *************************************/
    pType allocate();

		/************************************
		* Return true if the buffer is empty
		* ie: if no more element can be
		* allocated. This is thread-safe
		*************************************/
		bool empty();

#ifdef ENABLE_TEST_UNIT
class Test : public Test_aclass< PC_buffer<T> >
    {
    public:
        void tests();
    };
#endif //ENABLE_TEST_UNIT
private:
    /************************************
    * return the Buffer_element into the
    * free elements list. The function is
    * thread-safe an may unblock the
    * "allocate" function.
    *************************************/
    void release(pType element);

    Condition m_freequeuecond;

    // queue of the currently free Buffer_elements
    std::queue<pType> m_freequeue;

    // vector of all the allocated Buffer_elements
    std::vector<pType> m_vectorelements;

    // A static counter to give each buffer
    // an unique id.
    static int sid;
    int mid;

   PC_buffer(const PC_buffer<T>&);
};

////////////////// IMPLEMENTATION (I Hate c++ template) ///////////////
template<class T> int PC_buffer<T>::sid = 0;


template<class T> PC_buffer<T>::PC_buffer(const PC_buffer<T>&)
{
    MASSERT(false && "Not implemented");
}

template<class T> PC_buffer<T>::PC_buffer(unsigned int numelements, Allocator<T>* allocator )
{
    mid = sid++;
    for (unsigned int i=0;i<numelements;i++)
    {
        T* tmp = allocator->allocate();
        pType element = new Type(tmp);
        m_freequeue.push( element );
        m_vectorelements.push_back( element );
    }
}

template<class T> PC_buffer<T>::PC_buffer(unsigned int numelements )
{
		Default_allocator<T> allocator;
    mid = sid++;
    for (unsigned int i=0;i<numelements;i++)
    {
        T* tmp = allocator.allocate();
        pType element = new Type(tmp);
        m_freequeue.push( element );
        m_vectorelements.push_back( element );
    }
}

template<class T> PC_buffer<T>::PC_buffer(unsigned int numelements, Allocator<T>& allocator )
{
    mid = sid++;
    for (unsigned int i=0;i<numelements;i++)
    {
        T* tmp = allocator.allocate();
        pType element = new Type(tmp);
        m_freequeue.push( element );
        m_vectorelements.push_back( element );
    }
}

template<class T>
size_t PC_buffer<T>::number_free_element()
{
    RAIIMutex rc(m_freequeuecond);
    return m_freequeue.size();
}

// Blocking allocation of an element
template<class T>
typename PC_buffer<T>::pType PC_buffer<T>::allocate()
{
    RAIIMutex rc(m_freequeuecond);
    // use a while loop instead of an if to avoid
    // the spurious signal waking up.
    while ( m_freequeue.size() == 0 )
    {
        m_freequeuecond.wait();
    }

    pType element = m_freequeue.front();
    m_freequeue.pop();

    element->set_owner(this);
    return element;
}

template<class T>
void PC_buffer<T>::release(pType element)
{
    MASSERT( element != NULL );

    RAIIMutex rc(m_freequeuecond);
    m_freequeue.push(element);

    MASSERT( m_freequeue.size() <= m_vectorelements.size() );
    if ( m_freequeue.size() == 1  )
    {
        m_freequeuecond.signal();
    }
}

template<class T>
bool PC_buffer<T>::empty()
{
    RAIIMutex rc(m_freequeuecond);
		return m_freequeue.size() == 0;
}

template<class T>
PC_buffer<T>::Buffer_element::Buffer_element(pType data)
{
    MASSERT( data  != NULL );

    m_data = data;
    m_owner = NULL;
}

template<class T>
typename PC_buffer<T>::Buffer_element::pType PC_buffer<T>::Buffer_element::data()
{
    return m_data;
}

template<class T>
void PC_buffer<T>::Buffer_element::set_owner( PC_buffer<T>* owner )
{
		MASSERT( owner != NULL && "NULL pointer exception" );
		MASSERT( m_owner == NULL && "Allocating an already allocated object" );
		m_owner = owner;
}

template<class T>
void PC_buffer<T>::Buffer_element::release()
{
		if( m_owner == NULL ) MTHROW("double release is forbidden");
    m_owner->release(this);
    m_owner = NULL;
}

#ifdef ENABLE_TEST_UNIT
template<class T>
void PC_buffer<T>::Test::tests()
{
		PC_buffer<T> t(1);
		pType elem = NULL;

		// This part is testing the public interface
		TEST_ASSERT( t.empty() == false );
		TEST_EXCEPTION_NTHROW( elem = t.allocate() );
    TEST_ASSERT( t.empty() == true );
    TEST_EXCEPTION_NTHROW( elem->release() );
		TEST_ASSERT( t.empty() == false );
		TEST_EXCEPTION_THROW( elem->release() );
		TEST_ASSERT( t.empty() == false );

		// This part is testing the private interface
    TEST_EXCEPTION_THROW( t.release(NULL) );
}
#endif // ENABLE_TEST_UNIT

#endif // BUFFER_H
