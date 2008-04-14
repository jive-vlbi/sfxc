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
 *   - The basic definition for exception object
 */
#ifndef EXCEPTION_COMMON_HH_INCLUDED
#define EXCEPTION_COMMON_HH_INCLUDED

#include <string.h>
#include <stdexcept>
#include "backtrace.h"

/*********************************************
* @class Exception
* @desc Implement the basic exception
* facility.
*
**********************************************/
class Exception : public std::exception {
public:
  /******************************************
  * Create a new exception, the backtrace
  * has to be provided externally. As this can
  * be tiring to type evertime, use the macro
  * MTHROW when you want to throw an exception.
  *******************************************/
  Exception(const std::string& message, Backtrace& bt);
  virtual ~Exception() throw ();

  /******************************************
    * return the backtrace of the exception
    * ideally this should include all the function
    * call stack. As this is compiler-depend some
    * shortcomming are provided that simply return
    * the location in the source-code where the
    * exception was risen.
    *******************************************/
  Backtrace& backtrace();

  /******************************************
  * return a string containing the type of
  * exception in an human readable format.
  *******************************************/
  virtual const std::string type();

  /******************************************
  * return the exception message.
  *******************************************/
  virtual const std::string message();

  /******************************************
    * return the exception message.
    *******************************************/
  virtual const char* what() const throw() {
    return message_.c_str();
  }

  friend std::ostream& operator<<(std::ostream& out, Exception& exception);
protected:
  std::string message_;
  Backtrace backtrace_;
};

//Backtrace bt(__PRETTY_FUNCTION__);

#define CHECK_PARAMETER( condi )     \
    if ( !(condi) )  \
    {                                       \
        Backtrace bt;      \
        throw Exception("PARAMETER CONSTRAINT VIOLATION: ", bt);  \
    }



/******************************************
* Use this in your code in order to allow
* fast switching from assertion  to
* exception throwing.
*******************************************/
#define MASSERT( condi )     \
    if ( !(condi) )  \
    {                                       \
        Backtrace bt;      \
        std::cout << bt << std::endl; sleep(1); \
        throw Exception(#condi, bt);  \
    }

/******************************************
* One-line exception throw. Use this in your
* code for throwing an exception with
* the required backtrace object.
*******************************************/
#define MTHROW( text )    \
    {                                       \
        Backtrace bt;      \
        throw Exception(text, bt);  \
    }

/******************************************
* One-line debugging with color to display
* a variable with its name, type and
* content. Maybe usefull in a templated
* code.
*******************************************/
#define F_DEBUG(val) \
    { \
        std::cout << "\033[31mDEBUG: name[" << #val << "] type[" << get_type_name(val) << "], value[" << val << "]\033[30m" << std::endl; \
    }




#endif // EXCEPTION_HH_INCLUDED
