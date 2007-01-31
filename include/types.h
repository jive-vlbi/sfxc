/*
CVS keywords
$Author$
$Date$
$Name$
$Revision$
$Source$

Author     : NGH Kruithof
StartDate  : 20061101
Last change: 20061124
*/

/** 
    Definitions of common types.
    
    Author: Nico Kruithof <Kruithof@jive.nl>
**/

#ifndef TYPES_H
#define TYPES_H

#include "../config.h"

// For using large files:
#undef  _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE 1
#undef  _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE 1
#undef  _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64

// typedef long INT64;
#if SIZEOF_LONG==8
  #define INT64 long 
  #define UINT64 unsigned long
  #define MPI_INT64  MPI_LONG
  #define MPI_UINT64 MPI_UNSIGNED_LONG
#elif SIZEOF_LONG_LONG==8
  #define INT64 long long 
  #define UINT64 unsigned long long
  #define MPI_INT64  MPI_LONG_LONG_INT
  #define MPI_UINT64 MPI_UNSIGNED_LONG_LONG_INT
#elif SIZEOF_INT==8
  #define INT64 int 
  #define UINT64 unsigned int
  #define MPI_INT64  MPI_INT
  #define MPI_UINT64 MPI_UNSIGNED_INT
#else
  NGHK: No 64 bits type found ...
#endif

#if SIZEOF_SHORT==4
  #define INT32  short 
  #define UINT32 unsigned short
  #define MPI_INT32  MPI_SHORT
  #define MPI_UINT32 MPI_UNSIGNED_SHORT
#elif SIZEOF_INT==4
  #define INT32  int 
  #define UINT32 unsigned int
  #define MPI_INT32  MPI_INT
  #define MPI_UINT32 MPI_UNSIGNED_INT
#elif SIZEOF_LONG==4
  #define INT32  long 
  #define UINT32 unsigned long
  #define MPI_INT32  MPI_LONG_INT
  #define MPI_UINT32 MPI_UNSIGNED_LONG_INT
#elif SIZEOF_LONG_LONG==4
  #define INT32  long long 
  #define UINT32 unsigned long long
  #define MPI_INT32  MPI_LONG_INT
  #define MPI_UINT32 MPI_UNSIGNED_LONG_INT
#else
  NGHK: No 32 bits type found ...
#endif

/** 
    \defgroup ImportantClasses Important classes
 **/
#endif // TYPES_H
