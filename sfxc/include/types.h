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


#if SIZEOF_LONG_LONG==8
#define INT64 long long 
#define UINT64 unsigned long long
#elif SIZEOF_LONG==8
#define INT64 long 
#define UINT64 unsigned long
// typedef long INT64;
#elif SIZEOF_INT==8
#define INT64 int 
#define UINT64 unsigned int
// typedef int INT64;
#else
NGHK: No 64 bits type found ...
#endif

#if SIZEOF_LONG_LONG==4
#define INT32 long long 
#define UINT32 unsigned long long
#elif SIZEOF_LONG==4
#define INT32 long 
#define UINT32 unsigned long
#elif SIZEOF_INT==4
#define INT32 int 
#define UINT32 unsigned int
#else
NGHK: No 32 bits type found ...
#endif

enum MPI_TAG {
  /// General initialisation channel
  MPI_TAG_INIT =0,       
  /// General communication channel
  MPI_TAG_COMMUNICATION,
  /// Add an input stream to a correlator node, using MPI (not in use)
  MPI_TAG_ADD_INPUT_MPI,
  /// Add an input stream to a correlator node, using TCP
  MPI_TAG_ADD_INPUT_TCP,
  /// Add a channel to a correlator node
  MPI_TAG_ADD_CHANNEL,   
  /// Notify an input reader about a correlator node
  MPI_TAG_ADD_CORRELATOR_NODE_TO_READER_TCP,

  MPI_TAG_ERROR
};

// MPI_MSG are always sent over MPI_TAG_COMMUNICATION
enum MPI_MSG {
  /// You should be a node to which data is sent
  MPI_INPUT_NODE = 0,
  /// You should be a node that can correlate data
  MPI_CORRELATOR_NODE,

  /// The correlator node is ready to process data
  MPI_CORRELATOR_READY,

  /// The correlation node is ready to process data
  MPI_CORRELATION_READY,
  MPI_MSG_ERROR
};
#endif // TYPES_H
