/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 */
#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED


#include <string>
typedef std::string   String;
typedef std::string* pString;
typedef std::string& rString;

#include <vector>
typedef std::vector<String> Vector_string;
typedef std::vector<pString> Vector_pstring;

#include <complex>
typedef std::complex<double>   Complex;


// Conversion from string to anytype supported by istrinstream
#include <sstream>
/*template<class T>
T& operator<<(T& dest, String& s)
{
 std::stringstream ss(s);
  dest << ss;
 return dest;
}*/

int& operator<<(int& dest, const String& s);

template<class T>
T& operator<<(String& s, T& elem) {
  std::stringstream ss;
  ss << elem;
  s.append( ss.str() );
  return s;
}


#endif // COMMON_H_INCLUDED

