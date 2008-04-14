/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)

 * Copyright (c) 2007 University of Amsterdam (Netherlands)

 * All rights reserved.

 *

 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007

 *            Damien Marchal <dmarchal@science.uva.nl>, 2007

 *

 *

 * This file is part of:

 *   - common library

 * This file contains:

 *   - RAIIMutex class declaration and definition

 */
#ifndef RAIIMUTEX_H
#define RAIIMUTEX_H

#include "mutex.h"

/*****************************************
*
* @class RAIIMutex
* @desc Handle the locking/unlocking
* of the mutexes in the constructor
* destructor of the class. You should
* always use this to insure the a locked
* mutex will be safely release if an
* exception occurs.
******************************************/
class RAIIMutex {
public:
  /************************************

  * Create the objec and lock the mutex

  * given in parameters

  *************************************/
  inline RAIIMutex(Mutex& mutex);

  /************************************
  * Destroy the object and unlock the
  * previously locked mutex.
  *************************************/
  inline ~RAIIMutex();

private:
  Mutex& mutex_;
};

////////////////////////////////////
// Implementation of RAIIMutex::*
////////////////////////////////////
inline RAIIMutex::RAIIMutex(Mutex& mutex) :
    mutex_(mutex) {
  //std::cout << __PRETTY_FUNCTION__ << std::endl;
  mutex_.lock();
}

inline RAIIMutex::~RAIIMutex() {
  //std::cout << __PRETTY_FUNCTION__ << std::endl;
  mutex_.unlock();
}

#endif // RAIIMUTEX_H
