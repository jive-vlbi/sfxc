/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 *
 *  This file is part of the unitary test framework
 *  This file contains:
 *    - declaration of the interface every "testable"
 *      object should implement
 */
#ifndef TESTABLE_H
#define TESTABLE_H


class Testable
{
	public:
    virtual ~Testable();
    virtual void test() = 0;
		virtual void tests() = 0;
};

typedef Testable* pTestable;
typedef Testable& rTestable;


#endif // TESTABLE_H
