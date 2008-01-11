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
 * @class Memory_pool
 * @desc A Memory_pool is used to control
 * ownership of objects T. An object
 * can be allocated from the buffer,
 * manipulated and then destroyed
 * (queued back) into the buffer.
 * allocation and release of the data
 * stored in the buffer are with an
 * O(1) complexity.
 * Futur plan:
 *  we should be able to specified
 *  which strategy the Memory_pool could
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
class Memory_pool {
public:
  /************************************
   * @class Buffer_element
   * @desc This is the object return by
   * the buffer.
   *************************************/
  class Buffer_element {
  public:
    Buffer_element();
    Buffer_element(const Buffer_element& src);

    /***************************************
     * Typedef to the real object type
     * accessible using  this Buffer_element
     ****************************************/
    typedef T Type;
    typedef Type value_type;

    /***************************************
     * Returns a  to the object stores in
     * the buffer element.
     ****************************************/
    Type& data() const;
    Type& operator*() const;
    Type *operator->() const;

    /***************************************
     * The buffer_element is release and goes
     * back into the Memory_pool. The data should
     * not be access further. It is user responsability
     * to not use the object return data() after
     * the release call.
     * throw an exception in case of double
     * release.
     ****************************************/
    void release();
    bool released() const { return m_data == NULL; }

    static Buffer_element None;

    bool operator==(const Buffer_element& c) const {
      return m_data == c.m_data && m_owner == c.m_owner;
    }
    bool operator!=(const Buffer_element& c) const {
      return ! (*this == c);
    }
  private:
    friend class Memory_pool<T>;
    Buffer_element(Type* data, Memory_pool<T>* owner);

    // Pointer to the data
    T* m_data;

    // Pointer to the buffer that allocated the data
    Memory_pool<T>* m_owner;
  };

  /***************************************
   * Typedef to the real object type
   * accessible using  this Memory_pool
   ****************************************/
  typedef Buffer_element Element;

  /*****************************************
   * Construct a Memory_pool object containing
   * numelements that are allocated using the
   * provided allocator.
   *****************************************/
  Memory_pool(unsigned int numelements=10);
  Memory_pool(unsigned int numelements, Allocator<T>* allocator );
  Memory_pool(unsigned int numelements, Allocator<T>& allocator );

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
  Element allocate();

  /************************************
   * Return true if the buffer is empty
   * ie: if no more element can be
   * allocated. This is thread-safe
   *************************************/
  bool empty();

#ifdef ENABLE_TEST_UNIT

class Test : public Test_aclass< Memory_pool<T> > {
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
  void release(Element& element);

  Condition m_freequeuecond;

  // queue of the currently free Buffer_elements
  std::queue<T*> m_freequeue;

  // vector of all the allocated Buffer_elements
  std::vector<T*> m_vectorelements;

  // A static counter to give each buffer
  // an unique id.
  static int sid;
  int mid;

  Memory_pool(const Memory_pool<T>&);
};

////////////////// IMPLEMENTATION (I Hate c++ template) ///////////////
template<class T>
int Memory_pool<T>::sid = 0;
template<class T>
typename Memory_pool<T>::Element Memory_pool<T>::Element::None;

template<class T>
Memory_pool<T>::Memory_pool(const Memory_pool<T>&) {
  MASSERT(false && "Not implemented");
}

template<class T>
Memory_pool<T>::Memory_pool(unsigned int numelements, Allocator<T>* allocator ) {
  mid = sid++;
  for (unsigned int i=0;i<numelements;i++) {
    T* element = allocator->allocate();
    //pType element = new Type(tmp, *this);
    m_freequeue.push( element );
    m_vectorelements.push_back( element );
  }
}

template<class T>
Memory_pool<T>::Memory_pool(unsigned int numelements ) {
  Default_allocator<T> allocator;
  mid = sid++;
  for (unsigned int i=0;i<numelements;i++) {
    T* tmp = allocator.allocate();
    //pType element = new Type(tmp, *this);
    m_freequeue.push( tmp );
    m_vectorelements.push_back( tmp );
  }
}

template<class T>
Memory_pool<T>::Memory_pool(unsigned int numelements, Allocator<T>& allocator ) {
  mid = sid++;
  for (unsigned int i=0;i<numelements;i++) {
    T* tmp = allocator.allocate();
    //pType element = new Type(tmp, *this);
    m_freequeue.push( tmp );
    m_vectorelements.push_back( tmp );
  }
}

template<class T>
size_t Memory_pool<T>::number_free_element() {
  RAIIMutex rc(m_freequeuecond);
  return m_freequeue.size();
}

// Blocking allocation of an element
template<class T>
typename Memory_pool<T>::Element Memory_pool<T>::allocate() {
  RAIIMutex rc(m_freequeuecond);
  // use a while loop instead of an if to avoid
  // the spurious signal waking up.
  while ( m_freequeue.size() == 0 ) {
    m_freequeuecond.wait();
  }

  T* element = m_freequeue.front();
  m_freequeue.pop();

  //element->set_owner(this);
  return Element(element, this);
}

template<class T>
void Memory_pool<T>::release(Element& element) {
  RAIIMutex rc(m_freequeuecond);
  m_freequeue.push(element.m_data);

  MASSERT( m_freequeue.size() <= m_vectorelements.size() );
  if ( m_freequeue.size() == 1  ) {
    m_freequeuecond.signal();
  }
}

template<class T>
bool Memory_pool<T>::empty() {
  RAIIMutex rc(m_freequeuecond);
  return m_freequeue.size() == 0;
}


template<class T>
Memory_pool<T>::Buffer_element::Buffer_element() {
  m_data = NULL;
  m_owner = NULL;
}

template<class T>
Memory_pool<T>::Buffer_element::Buffer_element(const Buffer_element& src) {
  m_owner = src.m_owner;
  m_data = src.m_data;
}

template<class T>
Memory_pool<T>::Buffer_element::Buffer_element(Type* data, Memory_pool<T>* owner) :
m_owner(owner) {
  MASSERT( data  != NULL );
  m_data = data;
}

template<class T>
typename Memory_pool<T>::Element::Type& Memory_pool<T>::Buffer_element::data() const {
  MASSERT(m_data != NULL);
  return *m_data;
}

template<class T>
typename Memory_pool<T>::Element::Type& 
Memory_pool<T>::Buffer_element::operator*() const {
  MASSERT(m_data != NULL);
  return *m_data;
}

template<class T>
typename Memory_pool<T>::Element::Type * 
Memory_pool<T>::Buffer_element::operator->() const {
  return &operator*();
}

template<class T>
void Memory_pool<T>::Buffer_element::release() {
  if( m_owner == NULL )
    MTHROW("double release is forbidden");
  m_owner->release( *this );
  m_owner = NULL;
}

#ifdef ENABLE_TEST_UNIT
template<class T>
void Memory_pool<T>::Test::tests() {
  Memory_pool<T> t(1);
  Element elem = Element::None;

  // This part is testing the public interface
  TEST_ASSERT( (elem == Element::None) );
  TEST_ASSERT( t.empty() == false );
  TEST_EXCEPTION_NTHROW( elem = t.allocate() );
  TEST_ASSERT( elem != Element::None );
  TEST_ASSERT( t.empty() == true );
  TEST_EXCEPTION_NTHROW( elem.release() );
  TEST_ASSERT( t.empty() == false );
  TEST_EXCEPTION_THROW( elem.release() );
  TEST_ASSERT( t.empty() == false );

  // This part is testing the private interface
  //TEST_EXCEPTION_THROW( t.release(NULL) );
}
#endif // ENABLE_TEST_UNIT

#endif // BUFFER_H
