/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Ruud Oerlemans <Oerlemans@JIVE.nl>, 2007
 * 
 * $Id$
 *
 * A general header file with contants to be included in sfxc01.cc
 * and other source files
 */

#ifndef CONSTPRMS_H
#define CONSTPRMS_H

#include <string>

const int   lineLength    =     256; //maximum allowed line length in input file
const int   strLength     =     256; //maximum allowed string length
const int   NprocessesMax =      50; //maximum number of parallel processes
const int   NstationsMax  =      50; //maximum number of stations
const int   BufTime       =   16384; //delta time for Bufs in micro seconds
                                     //must be power of 2
                                     //2*BufTime > deltaTR
                                     //deltaTR=Earthradius/SpeedOfLight
                                     //deltaTR=6378000/299792458*1e6=21275 microseconds

enum Datatypes {
  DATATYPE_UNDEFINED = -1,
  DATATYPE_MK4
};

#endif // CONSTPRMS_H
