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
 *   - An utility class to backtrace the program.
 *     this is useful to print a nice debugging trace
 *     this use a gcc-specific part of the api. Non portable
 */
#ifndef BACKTRACE_H
#define BACKTRACE_H

#if !defined(WIN32)
#include <signal.h>
#endif
#if !defined(WIN32)
#include <execinfo.h>
#include <unistd.h>
#endif
#ifdef __GNUC__
#include <cxxabi.h>
#endif

/*
#ifdef ENABLE_TEST_UNIT
#include "Test_unit.h"
#endif // ENABLE_TEST_UNIT
*/

#include "common.h"

class Backtrace
{
    //String trace_;
		Vector_string trace_;

public:
	  Backtrace();
    Backtrace(const Backtrace& bt);
    Backtrace(const String& trace);

    friend std::ostream& operator<<(std::ostream&, Backtrace& backtrace);

		/*
		#ifdef ENABLE_TEST_UNIT
		class Test : public Test_aclass<Backtrace>
		{
					public:
						void tests();
			};
		#endif // ENABLE_TEST_UNIT
		*/
};

#endif // BACKTRACE_H
