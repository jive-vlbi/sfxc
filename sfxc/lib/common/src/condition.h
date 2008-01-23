/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 *
 * This file is part of:
 *   - xxx
 * This file contains:
 *   - xxx
 */
#ifndef CONDITION_H
#define CONDITION_H

#include "mutex.h"

/**************************************
* @class Condition
* @desc Implementation on a condition
* barrier.
* This class is simply a wrapper around
* the pthread_cond object.
***************************************/
class Condition : public Mutex
{
    pthread_cond_t condition_;
public:
    /************************************
    * Create and initialize the condition
    *************************************/
    Condition()
    {
        pthread_cond_init( &condition_, NULL);
    }

    /************************************
    * Wait until somebody signal that the
    * condition may have changed. Condition
    * has to be recheck after the wait...
    * search for cond/wait invalid wakeup
    * in google.
    *************************************/
    inline void wait()
    {
        pthread_cond_wait( &condition_, &mutex_ );
    }

    /************************************
    * Signal one of the waiters that the
    * condition may have changed.
    *************************************/
    inline void signal()
    {
        pthread_cond_signal( &condition_ );
    }

    /************************************
    * Signal to all of the waiters that
    * the condition may have changed.
    *************************************/
    inline void broadcast()
    {
        pthread_cond_broadcast( &condition_ );
    }
};



#endif // CONDITION_H
