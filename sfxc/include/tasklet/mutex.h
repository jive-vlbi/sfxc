/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 *
 *  This file contains:
 *     - a class Mutex and its implementation
 *     - a class RAIIMutex and its implementation
 */
#ifndef MUTEX_SFXC_HH
#define MUTEX_SFXC_HH

#include <pthread.h>
#include <iostream>

/*
 *   @class Mutex
 *   @desc A mutex implementation.
 *   @author Damien Marchal
 */
class Mutex {
  static unsigned int s_id;
  unsigned int m_id;
public:
  inline Mutex();
  inline ~Mutex();

  // Same behavior as the POSIX pthread_mutex_t
  inline void lock ();
  inline void unlock();
protected:
  pthread_mutex_t m_mutex;
};


template<class T>
class MutexT : public Mutex {
  T* m_object;
public:
  MutexT() { }

  inline void lock () {
    //std::cout << "Locking mutex around: " << GetTypeName(*m_object) << std::endl;
    Mutex::lock ();
  }

  inline void unlock() {
    //std::cout << "Un;ocking mutex around: " << GetTypeName(*m_object) << std::endl;
    Mutex::unlock();
  }
};

class RAIIMutex {
public:
  inline RAIIMutex(Mutex& mutex);
  inline ~RAIIMutex();

private:
  Mutex& m_mutex;
};

////////////////////////////////////
// Implementation of Mutex::*
////////////////////////////////////
inline Mutex::Mutex() {
  //std::cout << "Mutex init" << std::endl;
  pthread_mutex_init(&m_mutex, NULL);
  m_id = s_id++;
}

inline Mutex::~Mutex() {
  //std::cout << "Mutex destroy" << std::endl;
  pthread_mutex_destroy( &m_mutex );
}

inline void Mutex::lock () {
  //std::cout << "Locking" << m_id << std::endl;
  pthread_mutex_lock( &m_mutex );
}

inline void Mutex::unlock() {
  //std::cout << "Mutex unlock" << std::endl;
  pthread_mutex_unlock( &m_mutex );
}



////////////////////////////////////
// Implementation of RAIIMutex::*
////////////////////////////////////
inline RAIIMutex::RAIIMutex(Mutex& mutex) :
    m_mutex(mutex) {
  //std::cout << __PRETTY_FUNCTION__ << std::endl;
  m_mutex.lock();
}

inline RAIIMutex::~RAIIMutex() {
  //std::cout << __PRETTY_FUNCTION__ << std::endl;
  m_mutex.unlock();
}



template<class T>
class RAIITrace {
public:
  RAIITrace(T& t) :m_t(t) {
    std::cout << "ENTER: " << GetTypeName(t) << std::endl;
  }

  ~RAIITrace() {
    std::cout << "LEAVING: " << GetTypeName(m_t) << std::endl;
  }

private:
  T& m_t;
};


class Condition : public Mutex {
  pthread_cond_t m_condition;
public:
  Condition() {
    pthread_cond_init( &m_condition, NULL);
  }

  inline void wait() {
    pthread_cond_wait( &m_condition, &m_mutex );
  }

  inline void signal() {
    pthread_cond_signal( &m_condition );
  }

  inline void broadcast() {
    pthread_cond_broadcast( &m_condition );
  }
};

#endif // MUTEX_HH
