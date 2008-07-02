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
 *   - the definition of the Demangler object.
 */
#include <string>
#include<iostream>
#include <cassert>

// This is GCC_SPECIFIC
#include<cxxabi.h>
#include "demangler.h"
#include "exception_common.h"

Demangler::Demangler(const char* name) {
  if (name == NULL ) {
    //MTHROW("NULL pointer");
		assert(false && "Null pointer");
  } else {
    int status;
    buffer_ = abi::__cxa_demangle( name, NULL, NULL, &status );

#if 0
    switch ( status ) {
    case  0:
      break
    case -1:
      MTHROW("Memory allocation failed during demangling");
    case -2:
      MTHROW("This is not a valid mangled name ["+String(name)+"]");
    case -3:
      MTHROW("One of the argument is invalid");
    }
#endif

    switch ( status ) {
    case  0:
      name_ = buffer_;
      free(buffer_);
      buffer_=NULL;
      break;
    default:
      name_ = name;
      break;
    }
  }

}

Demangler::~Demangler() {
  if ( buffer_ ) delete[] buffer_;
}

std::ostream& operator<<(std::ostream& str, Demangler& dem ) {
  str << dem.name_;
  return str;
}

std::string& Demangler::value() {
  return name_;
}

/*
void Demangler::Test::tests()
{
 float x;
 Demangler demangler("_Z12totobou");
 TEST_ASSERT( get_type_name(demangler) == "Demangler" );
 TEST_ASSERT( get_type_name(x) == "float" );
 TEST_ASSERT( get_type_name<Demangler>() == "Demangler" );
}*/
