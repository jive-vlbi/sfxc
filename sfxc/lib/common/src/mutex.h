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
 */
#ifndef MUTEX_SFXC_HH
#define MUTEX_SFXC_HH

#include <pthread.h>
#include <iostream>

/*****************************************
 *   @class Mutex
 *   @desc A mutex implementation.
 *   @author Damien Marchal
 *   This is not a recursive mutex, please
 *   consult pthread documentation to learn
 *   the difference.
 *****************************************/
class Mutex
{
public:

    inline Mutex();
    inline ~Mutex();

    // Same behavior as the POSIX pthread_mutex_t
    inline void lock();
    inline void unlock();

protected:
    pthread_mutex_t mutex_;

private:
    // Each mutex has a fixed and unique id,
    unsigned int id_;
    static unsigned int s_ids_;
};

////////////////////////////////////
// Implementation of Mutex::*
////////////////////////////////////
inline Mutex::Mutex()
{
    //std::cout << "Mutex init" << std::endl;
    pthread_mutex_init(&mutex_, NULL);
    id_ = s_ids_++;
}

inline Mutex::~Mutex()
{
    //std::cout << "Mutex destroy" << std::endl;
    pthread_mutex_destroy( &mutex_ );
}

inline void Mutex::lock()
{
    //std::cout << "Locking" << m_id << std::endl;
    pthread_mutex_lock( &mutex_ );
}

inline void Mutex::unlock()
{
    //std::cout << "Mutex unlock" << std::endl;
    pthread_mutex_unlock( &mutex_ );
}

#endif // MUTEX_HH
