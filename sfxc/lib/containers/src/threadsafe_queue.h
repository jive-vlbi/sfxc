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
 *   - Declaration and definition of the pc_queue
 */
#ifndef PC_QUEUE_H
#define PC_QUEUE_H

#include <iostream>
#include <vector>
#include <queue>

#include "mutex.h"
#include "raiimutex.h"
#include "condition.h"
#include "exception_common.h"
#include "allocator.h"

#ifdef ENABLE_TEST_UNIT
#include "Test_unit.h"
#endif // ENABLE_TEST_UNIT

class QueueClosedException : public Exception
{
	public:
		QueueClosedException():Exception(""){};
};

/************************************************
* @class Threadsafe_queue
* @desc A queue implementation that is
* adequate for asynchrnous-thread-safe
* application. This class doesn't do any
* memory managment related to the object that
* are in the queue, please consult Memory_pool
* for a nice buffer implementation.
*
* Semantics:
*      - push is thread-safe and non-blocking.
*      - pop is thread-safe an blocking.
*      - pop_non_blocking is thread-safe and....
*      - empty is thread-safe non blocking
*
***********************************************/
template<class T>
class Threadsafe_queue {
public:
  typedef T     Type;
  typedef Type  value_type;

  Threadsafe_queue() { isclose_ = false; }
  virtual ~Threadsafe_queue() { close(); }

  void push( Type element ) {
		if( isclose_ )throw QueueClosedException();

    RAIIMutex rc(m_queuecond);
    m_queue.push(element);
    if( m_queue.size() != 0 ) m_queuecond.signal();
  }

  Type& front() {
    RAIIMutex rc(m_queuecond);
    while ( m_queue.size() == 0 ){
			if( isclose_ )throw QueueClosedException();
			m_queuecond.wait();
			if( isclose_ )throw QueueClosedException();
    }
    return m_queue.front();
  }

  void pop() {
    RAIIMutex rc(m_queuecond);
    while ( m_queue.size() == 0 ){
			 if( isclose_ )throw QueueClosedException();
    	 m_queuecond.wait();
			 if( isclose_ )throw QueueClosedException();
    }
    m_queue.pop();
  }

  Type front_and_pop() {
    RAIIMutex rc(m_queuecond);
    while ( m_queue.size() == 0 ){
			 if( isclose_ )throw QueueClosedException();
    	 m_queuecond.wait();
			 if( isclose_ )throw QueueClosedException();
    }
    Type element = m_queue.front();
    m_queue.pop();
    return element;
  }

  Type front_and_pop_non_blocking() {
    RAIIMutex rc(m_queuecond);
    if ( m_queue.size() == 0 ){
			if( isclose_ )throw QueueClosedException();
    	 MTHROW("Trying to pop from an empty queue.");
    }
    Type element = m_queue.front();
    m_queue.pop();
    return element;
  }

  bool empty() {
    RAIIMutex rc(m_queuecond);
    return m_queue.empty();
  }

  size_t size() {
    RAIIMutex rc(m_queuecond);
    return m_queue.size();
  }

	bool isclose(){ return isclose_;  }

	void close()
	{
    RAIIMutex rc(m_queuecond);

		/// the queue is closed
		isclose_=true;

		/// all the waiting classes now exit.
		m_queuecond.broadcast();
	}

#ifdef ENABLE_TEST_UNIT
class Test : public Test_aclass<Threadsafe_queue> {
  public:
    void tests();
  };
#endif // ENABLE_TEST_UNIT

private:
  std::queue<Type> m_queue;
  Condition m_queuecond;

  bool isclose_;
};

/////////////////// IMPLEMENTATION (I hate c++ template) ///////////////
#ifdef ENABLE_TEST_UNIT
template<class T>
void Threadsafe_queue<T>::Test::tests() {
  Threadsafe_queue<T*> queue;
  T obj;
  T* pobj=NULL;

  TEST_ASSERT( queue.empty() );
  TEST_EXCEPTION_THROW( queue.front_and_pop_non_blocking() );
  TEST_EXCEPTION_NTHROW( queue.push( &obj ) );
  TEST_EXCEPTION_NTHROW ( queue.push( NULL ) );
  TEST_ASSERT( !queue.empty() );
  TEST_EXCEPTION_NTHROW( pobj = queue.front_and_pop_non_blocking() );
  TEST_ASSERT( pobj == &obj );
  TEST_EXCEPTION_NTHROW( pobj = queue.front_and_pop_non_blocking() );
  TEST_ASSERT( pobj ==  NULL );
  TEST_ASSERT( queue.empty() );
  TEST_EXCEPTION_THROW( queue.front_and_pop_non_blocking() );
  TEST_ASSERT( queue.empty() );
  TEST_EXCEPTION_NTHROW( queue.push( &obj ) );
  TEST_ASSERT( !queue.empty() );
  TEST_EXCEPTION_NTHROW( queue.pop() );
  TEST_ASSERT( queue.empty() );
}
#endif // ENABLE_TEST_UNIT




#endif // PC_QUEUE_H
