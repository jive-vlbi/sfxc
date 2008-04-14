/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Damien Marchal <dmarchal@science.uva.nl>, 2007
 *            Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 *
 * This file is part of:
 *   - netlib library
 * This file contains:
 *   - Description of a Connexion_listener_thread object
 */
#ifndef CONNEXIONLISTENERTHREAD_H
#define CONNEXIONLISTENERTHREAD_H

#include <cassert>
#include "network.h"
#include "thread.h"
#include "connexion_listener.h"

/**************************************
* @class Connexion_thread_allocator
* @desc Abstract class that will allocate
* "service" threads in the context of a
* server-multiple/client architecture.
*
***************************************/
template<class T>
class Connexion_thread_allocator {
public:
  typedef T     Type;
  typedef Type* Type_ptr;

  virtual ~Connexion_thread_allocator() {}
  virtual Type_ptr allocate(int socket) = 0;
};


/**************************************
* @class Connexion_listener_thread
* @desc Abstract class to overide in
* order to implement a "listening"
* service.
*
***************************************/
template<class T>
class Connexion_listener_thread : public Thread {
public:
  typedef T   Type;
  typedef T*  Type_ptr;

  Connexion_listener_thread(Connexion_listener_ptr listener, Connexion_thread_allocator<T>* allocator);
  Connexion_listener_thread(Connexion_listener_ptr listener, Connexion_thread_allocator<T>& allocator);

  virtual ~Connexion_listener_thread();
  virtual void do_execute();
private:
  // Connexion allocatore to generate new connexion handler
  Connexion_thread_allocator<T>& allocator_;

  // The vector of active connexion handler...
  //TSVector< Type > m_vectorclients;

  // The port on which listen for connexion
  Connexion_listener_ptr listener_;
  bool isrunning_;
};

template<class T>
Connexion_listener_thread<T>::Connexion_listener_thread(Connexion_listener_ptr listener, Connexion_thread_allocator<T>* allocator) :
    allocator_(*allocator), listener_(listener) {
  assert(listener_ != NULL );
}

template<class T>
Connexion_listener_thread<T>::Connexion_listener_thread(Connexion_listener_ptr listener, Connexion_thread_allocator<T>& allocator) :
    allocator_(allocator), listener_(listener) {
  assert(listener_ != NULL );
}

template<class T>
Connexion_listener_thread<T>::~Connexion_listener_thread() {}

template<class T>
void Connexion_listener_thread<T>::do_execute() {
  isrunning_ = true;
  while ( isrunning_ ) {
    int socket = listener_->wait_connexion();
    Type_ptr writer = allocator_.allocate(socket);
    writer->start();
  }
}

#endif // CONNEXIONLISTENERTHREAD_H
