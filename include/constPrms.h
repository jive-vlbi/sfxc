/*
CVS keywords
$Author$
$Date$
$Name$
$Revision$
$Source$

A general header file with contants to be included in sfxc01.cc
and other source files

Author     : RHJ Oerlemans
StartDate  : 20060912
Last change: 20061114


*/
#ifndef CONSTPRMS_H
#define CONSTPRMS_H

#include "gen_defines.h"
#include <string>

const int   lineLength    =     256; //maximum allowed line length in input file
const int   strLength     =     256; //maximum allowed string length
const int   NprocessesMax =      50; //maximum number of parallel processes
const int   NstationsMax  =      50; //maximum number of stations
const int   BufTime       =   16384; //delta time for Bufs in micro seconds
                                     //2*BufTime > deltaTR
                                     //deltaTR=Earthradius/SpeedOfLight
                                     //deltaTR=6378000/299792458*1e6=21275 microseconds

enum Datatypes {
  DATATYPE_UNDEFINED = -1,
  DATATYPE_MK4
};

#endif // CONSTPRMS_H
