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
#ifndef PC_SIMPLE_MEMORY_POOL_H
#define PC_SIMPLE_MEMORY_POOL_H

#include <boost/shared_ptr.hpp>
#include <stack>
#include <vector>
#include "exception_common.h"

#include "raiimutex.h"
#include "condition.h"

#include "allocator.h"
#include "default_allocator.h"

#include "memory_pool.h"
#include "utils.h"

#ifdef ENABLE_TEST_UNIT
#include "Test_unit.h"
#endif // ENABLE_TEST_UNIT


/****************************************************
 * @class Simple_memory_pool
 * @desc A Simple_emory_pool
 *****************************************************/

template<class T>
class Simple_memory_pool {
public:

  class Resize_policy;

  typedef boost::shared_ptr<Resize_policy> PolicyPtr;
  typedef boost::shared_ptr<Allocator<T> > AllocatorPtr;

  /************************************
   * @class Resize_policy
   * @desc base interface that is
   * inherited in order to build more
   * complex resizing policies
   *************************************/
  class Resize_policy {
  public:
    static PolicyPtr create( Resize_policy_type type );

    virtual ~Resize_policy() {}
    virtual bool check_size(Simple_memory_pool<T>& pool, unsigned int newsize) = 0;
    virtual void resize(Simple_memory_pool<T>& pool) = 0 ;
  };



  /************************************
  * @class NoResize_policy
  * @desc A resize policy that do nothing
  * this is usefull if you want to prevent
  * a buffer to fill the complete memory.
  *************************************/
class NoResize_policy : public Resize_policy {
  public:

    static PolicyPtr create() {
      return PolicyPtr( new NoResize_policy() );
    }

    virtual ~NoResize_policy() {}
    inline bool check_size(Simple_memory_pool<T>& pool, unsigned int newsize) {
      return false;
    }
    inline void resize(Simple_memory_pool<T>& pool) {
      //std::cout << "Empty Buffer and a resize policy that do nothing !" << std::endl;
    }
  private:
    NoResize_policy() {}
  };

  /************************************
  * @class AutomaticResize_policy
  * @desc Increase the size of the
  * memory pool by a factor of 2 at
  * each call. There is no limit in the
  * increasement.
  *************************************/
class AutomaticResize_policy : public Resize_policy {
  public:
    static PolicyPtr create(int max_resize_count) {
      return PolicyPtr( new AutomaticResize_policy(max_resize_count) );
    }

    AutomaticResize_policy(int max_resize_count) {
      max_resize_count_ = max_resize_count;
      cur_resize_count_ = 0;
    }

    virtual ~AutomaticResize_policy() {}
    virtual bool check_size(Simple_memory_pool<T>& pool, unsigned int newsize) {
      if ( pool.empty_no_lock() ) {
        //MTHROW("Memory pool is empty")
        return true;
      }
      return false;
    }

    virtual void resize(Simple_memory_pool<T>& pool) {
      std::cout << "REQUEST FOR RESIZE THIS IS AN EXPERIMENTAL FEATURE:" << max_resize_count_ << std::endl;
      if ( cur_resize_count_ < max_resize_count_ ) {
        std::cout << "REQUEST FOR RESIZE NEW BUFFER SIZE IS:" << pool.size_no_lock()*2 << std::endl;
        pool.resize_no_lock(pool.size_no_lock()*2);
        cur_resize_count_++;
      } else {
        MTHROW("The memory pool is definitively empty.")
      }
    }
  private:
    int max_resize_count_;
    int cur_resize_count_;
  };

  /***************************************
   * Typedef to the real object type
   * accessible using  this Simple_memory_pool
   ****************************************/
  typedef T        value_type;
  typedef T        Element;
  typedef T*      pElement;

  /*****************************************
   * Construct a Simple_memory_pool object containing
   * numelements that are allocated using the
   * provided allocator.
   *****************************************/
  //Simple_memory_pool(unsigned int numelements=10, Allocator<T>* allocator=Default_allocator<T>(), Resize_policy& policy=NoResize_policy());
  //Simple_memory_pool(unsigned int numelements, Allocator<T>* allocator, Resize_policy& policy=NoResize_policy());
  Simple_memory_pool(unsigned int numelements, AllocatorPtr allocator= Default_allocator<T>::create(), PolicyPtr policy=AutomaticResize_policy::create(10));
  Simple_memory_pool(unsigned int numelements, Resize_policy_type type, AllocatorPtr allocator= Default_allocator<T>::create());

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
  pElement allocate();

  /************************************
   * Return true if the buffer is empty
   * ie: if no more element can be
   * allocated. This is thread-safe.
   * If the pool is empty it is possible
   * to resize manually the pool or
   * use an automatic resizing policy
   *************************************/
  bool empty();

  /************************************
   * Return true if all elements are in
   * the memory pool.
   *************************************/
  bool full();

  /************************************
  * Resize the memory_pool
  * to allocate more element without
  * blocking. This is thread-safe
  *************************************/
  void resize(unsigned int newsize);

  /************************************
  * Request for a memory pool increase
  * the pool then use its own internal
  * policy to increase its size.
  * (possibly refusing to increase
  *  the size)
  * If the pool is not increased.
  * an Exception is thrown.
  *************************************/
  void ask_increase(unsigned int newsize);

  /************************************
  * Return the number of total element
  * that can be allocated in the pool.
  * the function is thread-safe
  *************************************/
  unsigned int size();

  /************************************
  * Do not use these they are for
  * for internal use.
  *************************************/
  void resize_no_lock(unsigned int newsize);
  unsigned int size_no_lock();
  bool empty_no_lock();



  /************************************
   * return the element into the
   * free elements list.
   *************************************/
  void release(pElement element);

#ifdef ENABLE_TEST_UNIT
class Test : public Test_aclass< Simple_memory_pool<T> > {
  public:
    void tests();
  };
#endif //ENABLE_TEST_UNIT
private:
  Simple_memory_pool<T>() {}
  ;


  Condition m_freequeuecond;

  // queue of the currently free Buffer_elements
  std::stack<T*> m_freequeue;

  // vector of all the allocated Buffer_elements
  std::vector<T*> m_vectorelements;

  // A static counter to give each buffer
  // an unique id.
  static int sid;
  int mid;

  Simple_memory_pool(const Simple_memory_pool<T>&);
  PolicyPtr policy_;
  AllocatorPtr allocator_;
};

////////////////// IMPLEMENTATION (I Hate c++ template) ///////////////
template<class T>
int Simple_memory_pool<T>::sid = 0;

//template<class T>
//typename Simple_memory_pool<T>::pElement Simple_memory_pool<T>::Element::None;

template<class T>
Simple_memory_pool<T>::Simple_memory_pool(const Simple_memory_pool<T>&) {
  MASSERT(false && "Not implemented");
}

template<class T>
typename Simple_memory_pool<T>::PolicyPtr Simple_memory_pool<T>::Resize_policy::create( Resize_policy_type type ) {
  switch (type) {
  case AUTOMATIC_RESIZE:
    return AutomaticResize_policy::create(10);
  case NO_RESIZE:
    return NoResize_policy::create();
  default:
    MTHROW("Unknow resize policy");
  };
  MTHROW("This exception should never be risen");
}


template<class T>
Simple_memory_pool<T>::Simple_memory_pool(unsigned int numelements,
    Resize_policy_type type,
    AllocatorPtr allocator) :
    policy_( Resize_policy::create(type) ),
    allocator_(allocator) {
  mid = sid++;
  for (unsigned int i=0;i<numelements;i++) {
    T* tmp = allocator_->allocate();
    m_freequeue.push( tmp );
    m_vectorelements.push_back( tmp );
  }
}


template<class T>
Simple_memory_pool<T>::Simple_memory_pool(unsigned int numelements,
    AllocatorPtr allocator,
    PolicyPtr policy) :
    policy_(policy), allocator_(allocator) {
  mid = sid++;
  for (unsigned int i=0;i<numelements;i++) {
    T* tmp = allocator_->allocate();
    //pType element = new Type(tmp, *this);
    m_freequeue.push( tmp );
    m_vectorelements.push_back( tmp );
  }
}

template<class T>
void Simple_memory_pool<T>::resize(unsigned int newsize) {
  RAIIMutex rc(m_freequeuecond);
  if ( newsize > m_vectorelements.size() ) {
    for (unsigned int i=m_vectorelements.size();i<newsize;i++) {
      T* tmp = allocator_->allocate();
      m_freequeue.push( tmp );
      m_vectorelements.push_back( tmp );
    }
  }
}

template<class T>
void Simple_memory_pool<T>::resize_no_lock(unsigned int newsize) {
  if ( newsize > m_vectorelements.size() ) {
    for (unsigned int i=m_vectorelements.size();i<newsize;i++) {
      T* tmp = allocator_->allocate();
      m_freequeue.push( tmp );
      m_vectorelements.push_back( tmp );
    }
  } else {
    MTHROW("Unable to resize");
  }
}

template<class T>
void Simple_memory_pool<T>::ask_increase(unsigned int newsize) {
  RAIIMutex rc(m_freequeuecond);
  policy_->resize(newsize);
}

template<class T>
size_t Simple_memory_pool<T>::number_free_element() {
  RAIIMutex rc(m_freequeuecond);
  return m_freequeue.size();
}

// Blocking allocation of an element
template<class T>
typename Simple_memory_pool<T>::pElement Simple_memory_pool<T>::allocate() {
  RAIIMutex rc(m_freequeuecond);

  // if the queue has no free element...we check if we
  // resize it.
  if ( m_freequeue.size() == 0 ) {
    //std::cout << "ASK FRO RESIZE:" << policy_ << std::endl;
    policy_->resize(*this);
    //std::cout << "RESIZE TERMINATED:" << std::endl;
  }

  // use a while loop instead of an if to avoid
  // the spurious signal waking up.
  while ( m_freequeue.size() == 0 ) {
    m_freequeuecond.wait();
  }

  T* element = m_freequeue.top();
  m_freequeue.pop();

  //element->set_owner(this);
  return element;
}

template<class T>
void Simple_memory_pool<T>::release(pElement element) {
  RAIIMutex rc(m_freequeuecond);
  m_freequeue.push(element);

  MASSERT( m_freequeue.size() <= m_vectorelements.size() );
  if ( m_freequeue.size() == 1  ) {
    m_freequeuecond.signal();
  }
}

template<class T>
bool Simple_memory_pool<T>::empty() {
  RAIIMutex rc(m_freequeuecond);
  return m_freequeue.empty();
}

template<class T>
bool Simple_memory_pool<T>::full() {
  RAIIMutex rc(m_freequeuecond);
  return m_freequeue.size() == m_vectorelements.size();
}

template<class T>
bool Simple_memory_pool<T>::empty_no_lock() {
  return m_freequeue.empty();
}


template<class T>
unsigned int Simple_memory_pool<T>::size() {
  RAIIMutex rc(m_freequeuecond);
  return m_vectorelements.size();
}

template<class T>
unsigned int Simple_memory_pool<T>::size_no_lock() {
  return m_vectorelements.size();
}

#ifdef ENABLE_TEST_UNIT
template<class T>
void Simple_memory_pool<T>::Test::tests() {
  Simple_memory_pool<T> t(1, Default_allocator<int>::create(), Simple_memory_pool<int>::AutomaticResize_policy::create(1));
  pElement elem = NULL;

  // This part is testing the public interface
  TEST_ASSERT( (elem == NULL) );
  TEST_ASSERT( t.empty() == false );
  TEST_EXCEPTION_NTHROW( elem = t.allocate() );
  TEST_ASSERT( elem != NULL );
  TEST_ASSERT( t.empty() == true );
  TEST_EXCEPTION_NTHROW( elem.release() );
  TEST_ASSERT( t.empty() == false );
  TEST_EXCEPTION_THROW( elem.release() );
  TEST_ASSERT( t.empty() == false );

  // Testing the automatic allocation.
  TEST_EXCEPTION_NTHROW( elem = t.allocate() );
  TEST_EXCEPTION_THROW( elem.release() );
  TEST_EXCEPTION_THROW( elem.release() );
  TEST_ASSERT( t.empty() == false );
  TEST_EXCEPTION_NTHROW( elem = t.allocate() );
  TEST_EXCEPTION_NTHROW( elem = t.allocate() );
  TEST_EXCEPTION_THROW( elem = t.allocate() );


  // This part is testing the private interface
  //TEST_EXCEPTION_THROW( t.release(NULL) );
}
#endif // ENABLE_TEST_UNIT

#endif // SIMPLE_MEMORY_POOL_H
