/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 *
 * This file is part of:
 *   - common library, exception definition
 * This file contains:
 *   - Exception_indexoutofbound a exception that should be
 *     risen in case of using too large index for an array.
 */
#ifndef EXCEPTION_INDEXOUTOFBOUND_H
#define EXCEPTION_INDEXOUTOFBOUND_H

#include "exception_common.h"

/************************************************
*
* @class Exception_indexoutofbound
* @desc Use this kind of exception when an array
* is used with a too large index.
*
************************************************/
class Exception_indexoutofbound : public Exception
{
    public:
			/********************************************
			* Build an index out of bound exception
			*    size is the size of the array,
			*    index is the array element
			*********************************************/
			Exception_indexoutofbound(unsigned int index, unsigned int size, Backtrace& bt);

			virtual ~Exception_indexoutofbound() throw() ;

			/*******************************************
			* Inherited from Exception, return the
			* type of this exception as a string.
			*******************************************/
			virtual const std::string type();

		private:
			unsigned int index_;
			unsigned int size_;
};


/******************************************
* Throw an exception is an index is larger
* than the array size.
*******************************************/
#define CHECK_ARRAY_INDEX(index, array)     \
    if ( (index) >= (array) )  \
    {                                       \
        Backtrace bt(__PRETTY_FUNCTION__);      \
        throw IndexOutOfBoundException(index, array, bt);  \
    }

#endif // EXCEPTION_INDEXOUTOFBOUND_H
