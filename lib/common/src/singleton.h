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

 *   - singleton object

 */
#ifndef SINGLETON_HH_INCLUDED

#define SINGLETON_HH_INCLUDED


/*****************************************

* @class Singleton

* @desc Create an unique instance of

* a class that can be access using the
* instance member function.
* This class implies that the given class
* has an accessible default constructor.

******************************************/

template<class T>
class singleton {
public:
  static T& instance() {
    if ( s_obj_ == NULL ) s_obj_ = new T();
    return *s_obj_;
  }

private:
  static T *s_obj_;
};

template <typename T> T* singleton<T>::s_obj_ = NULL;



#endif // SINGLETON_HH_INCLUDED

