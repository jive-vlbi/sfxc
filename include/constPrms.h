/*
A general header file with contants to be included in sfxc01.cc
and other source files

Author     : RHJ Oerlemans
StartDate  : 12-09-2006
Last change: 12-09-2006


*/

#include "gen_defines.h"

const int   lineLength    =     256; //maximum allowed line length in input file
const int   strLength     =     256; //maximum allowed string length
const int   NcoresMax     =      50; //maximum number of stations
const int   NstationsMax  =      50; //maximum number of computational core
const int   BufTime       =   16384; //delta time for Bufs in micro seconds
                                     //2*BufTime > deltaTR
                                     //deltaTR=Earthradius/SOL
                                     //deltaTR=6378000/299792458*1e6=21275 microseconds
const string Mk4          =   "MK4";
