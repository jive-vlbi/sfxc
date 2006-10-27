/*
A general header file with contants to be included in sfxc.cc

Author     : RHJ Oerlemans
StartDate  : 12-09-2006
Last change: 12-09-2006


*/

#include "gen_defines.h"

const int   lineLength    =     256; //maximum allowed line length in input file
const int   strLength     =     256; //maximum allowed string length
const int   NcoresMax     =      50; //maximum number of stations
const int   NstationsMax  =      50; //maximum number of computational core
const INT64 BufSize       = 1048576; //buffersize for correlation (2^20)

const string Mk4          =   "MK4";
