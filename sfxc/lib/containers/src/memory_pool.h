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

#include <boost/shared_ptr.hpp>
#include <stack>
#include <vector>
#include "exception_common.h"

#include "raiimutex.h"
#include "condition.h"

#include "allocator.h"
#include "default_allocator.h"

#include "utils.h"

#ifdef ENABLE_TEST_UNIT
#include "Test_unit.h"
#endif // ENABLE_TEST_UNIT


/****************************************************
 * @class Memory_pool
 * @desc A Memory_pool is used to control
 * ownership of objects T. An object
 * can be allocated from the buffer,
 * manipulated and then destroyed
 * (queued back) into the buffer.
 * allocation and release of the data
 * stored in the buffer are with an
 * O(1) complexity.
 *
 * a call to allocate():
 *      - if not enough element
 *           use policy to resize
 *      - if still empty (the policy refuse to resize)
 *           block while a different thread dealloc
 *           an element
 *      return the element
 * a call to resize() will force a resize of the
 * pool.
 *
 * a call to ask_increase() will call the policy
 * to automatically resize the pool.
 *
 *****************************************************/
template<class T>
class Memory_pool {
public:

  /************************************
   * @class Resize_policy
   * @desc base interface that is 
   * inherited in order to build more 
   * complex resizing policies
   *************************************/
  class Resize_policy {
  public:


    virtual ~Resize_policy() {}
    virtual bool check_size(Memory_pool<T>& pool, unsigned int newsize) = 0;
    virtual void resize(Memory_pool<T>& pool) = 0 ;
  };

  typedef boost::shared_ptr<Resize_policy> PolicyPtr;
  typedef boost::shared_ptr<Allocator<T> > AllocatorPtr;


  /************************************
  * @class NoResize_policy
  * @desc A resize policy that do nothing
  * this is usefull if you want to prevent
  * a buffer to fill the complete memory.
  *************************************/
class NoResize_policy : public Resize_policy {
  public:

    PolicyPtr create(int max_resize_count=10) {
      return PolicyPtr( new NoResize_policy(max_resize_count) );
    }

    virtual ~NoResize_policy() {}
    bool check_size(Memory_pool<T>& pool, unsigned int newsize) {
      return false;
    }
    void resize(Memory_pool<T>& pool) {
      std::cout << "Empty Buffer and a resize policy that do nothing !" << std::endl;
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
    virtual bool check_size(Memory_pool<T>& pool, unsigned int newsize) {
      if ( pool.empty_no_lock() ) {
        //MTHROW("Memory pool is empty")
        return true;
      }
      return false;
    }

    virtual void resize(Memory_pool<T>& pool) {
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


  /************************************
   * @class Buffer_element
   * @desc This is the object return by
   * the buffer.
   *************************************/
  class Buffer_element {
  public:
    Buffer_element();
    Buffer_element(const Buffer_element& src);
    void operator=(const Buffer_element& src);
    ~Buffer_element();

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
     * Test whether the memory element points
     * to valid data.
     ****************************************/
    bool released() const {
      return m_data == NULL;
    }

    static Buffer_element None;

    bool operator==(const Buffer_element& c) const {
      return m_data == c.m_data && m_owner == c.m_owner;
    }
    bool operator!=(const Buffer_element& c) const {
      return ! (*this == c);
    }
  private:
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

    friend class Memory_pool<T>;
    Buffer_element(Type* data, Memory_pool<T>* owner);

    // Pointer to the data
    T* m_data;

    // Pointer to the buffer that allocated the data
    Memory_pool<T>* m_owner;

    // Reference counter to do an automatic release of the element
    int *m_reference_counter;
  };

  /***************************************
   * Typedef to the real object type
   * accessible using  this Memory_pool
   ****************************************/
  typedef Buffer_element Element;
  typedef Element        value_type             ;

  /*****************************************
   * Construct a Memory_pool object containing
   * numelements that are allocated using the
   * provided allocator.
   *****************************************/
  //Memory_pool(unsigned int numelements=10, Allocator<T>* allocator=Default_allocator<T>(), Resize_policy& policy=NoResize_policy());
  //Memory_pool(unsigned int numelements, Allocator<T>* allocator, Resize_policy& policy=NoResize_policy());
  Memory_pool(unsigned int numelements, AllocatorPtr allocator= Default_allocator<T>::create(), PolicyPtr policy=AutomaticResize_policy::create(10));

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


#ifdef ENABLE_TEST_UNIT
class Test : public Test_aclass< Memory_pool<T> > {
  public:
    void tests();
  };
#endif //ENABLE_TEST_UNIT
private:
  Memory_pool<T>() {}
  ;

  /************************************
   * return the Buffer_element into the
   * free elements list. The function is
   * thread-safe an may unblock the
   * "allocate" function.
   *************************************/
  void release(Element& element);

  Condition m_freequeuecond;

  // queue of the currently free Buffer_elements
  std::stack<T*> m_freequeue;

  // vector of all the allocated Buffer_elements
  std::vector<T*> m_vectorelements;

  // A static counter to give each buffer
  // an unique id.
  static int sid;
  int mid;

  Memory_pool(const Memory_pool<T>&);
  PolicyPtr policy_;
  AllocatorPtr allocator_;
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

/*
template<class T>
Memory_pool<T>::Memory_pool(unsigned int numelements, Allocator<T>* allocator, Resize_policy& policy ) :
 allocator_(allocator), policy_(policy)
 {
  mid = sid++;
  for (unsigned int i=0;i<numelements;i++) {
    T* element = allocator->allocate();
    //pType element = new Type(tmp, *this);
    m_freequeue.push( element );
    m_vectorelements.push_back( element );
  }
}
 
template<class T>
Memory_pool<T>::Memory_pool(unsigned int numelements, Allocator<T>& allocator, Resize_policy& policy ) :
  allocator_(allocator), policy_(policy)
{
  mid = sid++;
  for (unsigned int i=0;i<numelements;i++) {
    T* tmp = allocator_.allocate();
    //pType element = new Type(tmp, *this);
    m_freequeue.push( tmp );
    m_vectorelements.push_back( tmp );
  }
}*/

template<class T>
Memory_pool<T>::Memory_pool(unsigned int numelements, AllocatorPtr allocator, PolicyPtr policy) :
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
void Memory_pool<T>::resize(unsigned int newsize) {
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
void Memory_pool<T>::resize_no_lock(unsigned int newsize) {
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
void Memory_pool<T>::ask_increase(unsigned int newsize) {
  RAIIMutex rc(m_freequeuecond);
  policy_->resize(newsize);
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

  // if the queue has no free element...we check if we
  // resize it.
  if ( m_freequeue.size() == 0 ) {
    std::cout << "ASK FRO RESIZE:" << policy_ << std::endl;
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
  return m_freequeue.empty();
}

template<class T>
bool Memory_pool<T>::full() {
  RAIIMutex rc(m_freequeuecond);
  return m_freequeue.size() == m_vectorelements.size();
}

template<class T>
bool Memory_pool<T>::empty_no_lock() {
  return m_freequeue.empty();
}


template<class T>
unsigned int Memory_pool<T>::size() {
  RAIIMutex rc(m_freequeuecond);
  return m_vectorelements.size();
}

template<class T>
unsigned int Memory_pool<T>::size_no_lock() {
  return m_vectorelements.size();
}


template<class T>
Memory_pool<T>::Buffer_element::Buffer_element()
  : m_data(NULL), m_owner(NULL), m_reference_counter(new int(1)) {
}

template<class T>
Memory_pool<T>::Buffer_element::Buffer_element(Type* data, 
                                               Memory_pool<T>* owner) :
  m_data(data), m_owner(owner), m_reference_counter(new int(1)) {
  MASSERT( data  != NULL );
}

template<class T>
Memory_pool<T>::Buffer_element::Buffer_element(const Buffer_element& src)
: m_data(src.m_data), m_owner(src.m_owner), m_reference_counter(src.m_reference_counter) {
  (*m_reference_counter)++;
}

template<class T>
Memory_pool<T>::Buffer_element::~Buffer_element() {
  (*m_reference_counter)--;
  if ((*m_reference_counter)==0) {
    if ( m_owner != NULL )
      release();
    delete(m_reference_counter);
  }
}

template<class T>
void
Memory_pool<T>::Buffer_element::operator=(const Buffer_element& src) {
  // Remove the link to the previous buffer element
  (*m_reference_counter)--;
  assert((*m_reference_counter) >= 0);
  if ((*m_reference_counter)==0) {
    if ( m_owner != NULL )
      release();
    delete(m_reference_counter);
  }

  // Construct a link to the new element
  m_data = src.m_data;
  m_owner = src.m_owner;
  m_reference_counter = src.m_reference_counter;
  (*m_reference_counter)++;
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
  if ( m_owner == NULL )
    MTHROW("double release is forbidden");
  m_owner->release( *this );
  m_owner = NULL;
}

#ifdef ENABLE_TEST_UNIT
template<class T>
void Memory_pool<T>::Test::tests() {
  Memory_pool<T> t(1, Default_allocator<int>::create(), Memory_pool<int>::AutomaticResize_policy::create(1));
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

#endif // BUFFER_H
