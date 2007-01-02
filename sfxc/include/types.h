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

  /// Make an input node read data from file
  MPI_TAG_SET_INPUT_NODE_FILE,
  /// Add an input node obtaining data from a tcp connection
  MPI_TAG_SET_INPUT_NODE_TCP, // Not yet implemented
  /// Add a correlator node 
  MPI_TAG_SET_CORRELATOR_NODE,
  /// Add an input stream to a correlator node, using MPI (not in use)

  /// Make an output node write data to file
  MPI_TAG_SET_OUTPUT_NODE_FILE,

  // Set the station number for the commands following
  MPI_TAG_SET_STATION_NUMBER,
  // Send the start time for a correlate_node
  MPI_TAG_SET_START_TIME,

  // All settings are set for the correlation, start correlating
  MPI_TAG_START_CORRELATE_NODE,
  
  // Set the delay table for a data stream in a correlate node (DEPRECATED)
  MPI_TAG_SET_CONTROL_FILE,

  /// Notify an input node about a correlator node
  MPI_TAG_ADD_CORRELATOR_NODE,
  /// Ask for a way to forward data from an input reader to a corr_node
  MPI_TAG_ASK_COMMUNICATION_MEDIUM,

  /// Add a channel to a correlator node
  MPI_TAG_ADD_CHANNEL, // Not yet implemented
  /// Notify an input reader about a correlator node
  MPI_TAG_ADD_CORRELATOR_NODE_TO_READER_TCP,

  /// The data at a buffer stopped.
  MPI_MSG_DATASTREAM_EMPTY,
  
  /// A correlate node finished
  MPI_MSG_CORRELATE_ENDED,

  /// The correlation node is ready to process data
  MPI_MSG_CORRELATION_READY,
  
  /// Message sent to the message node (currently the controller node)
  MPI_MSG_TEXT_MESSAGE,

  MPI_TAG_ERROR
};

//// MPI_MSG are always sent over MPI_TAG_COMMUNICATION
//enum MPI_MSG {
////   /// You should be a node to which data is sent
////   MPI_INPUT_NODE = 0,
////   /// You should be a node that can correlate data
////   MPI_CORRELATOR_NODE,
//
//  /// The correlator node is ready to process data
//  MPI_CORRELATOR_READY,
//
//  MPI_MSG_ERROR
//};

//undef have to be before include <mpi.h>
// #ifdef SEEK_SET
// #undef SEEK_SET
// #endif
// #ifdef SEEK_END
// #undef SEEK_END
// #endif
// #ifdef SEEK_CUR
// #undef SEEK_CUR
// #endif
#include <mpi.h>

/** 
    \defgroup ImportantClasses Important classes
 **/
#endif // TYPES_H
