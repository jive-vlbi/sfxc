/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef SFXC_MPI_H
#define SFXC_MPI_H

#define RANK_MANAGER_NODE 0
#define RANK_LOG_NODE     1
#define RANK_OUTPUT_NODE  2

#define MPI_INT8   MPI_CHAR
#define MPI_UINT8  MPI_UNSIGNED_CHAR
#define MPI_INT16  MPI_SHORT
#define MPI_UINT16 MPI_UNSIGNED_SHORT
#define MPI_INT32  MPI_INT
#define MPI_UINT32 MPI_UNSIGNED
#define MPI_INT64  MPI_LONG_LONG

void start_node();

enum MPI_TAG {
  // INITIALISATION OF THE DIFFERENT TYPES OF NODES:
  //------------------------------------------------------------------------
  
  /** Add an input node
   * - MPI_INT32: number of the input node
   **/
  MPI_TAG_SET_INPUT_NODE,
  
  /** Add a correlator node
   * - MPI_INT32: no content
   **/
  MPI_TAG_SET_CORRELATOR_NODE,

  /** Add an output node
   * - MPI_INT32: no content
   **/
  MPI_TAG_SET_OUTPUT_NODE,
  
  /** Add a log node
   * - MPI_INT32: no content
   **/
  MPI_TAG_SET_LOG_NODE,

  /** Reply that the node is initialised
   * - MPI_INT32: no content
   **/
  MPI_TAG_NODE_INITIALISED,


  /** Get the status of the node
   * - MPI_INT32: status
   **/
  MPI_TAG_GET_STATUS,

  /** Set the messagelevel of the node
   * - MPI_INT32: message level
   **/
  MPI_TAG_SET_MESSAGELEVEL,
  
  // DATA COMMUNICATION, SET is for a single connection, ADD for multiple
  //--------------------------------------------------------------------------

  /** Create a data reader stream for incoming data using TCP
   * - INT32_t: stream number
   * - CHAR+: filename
   **/
  MPI_TAG_ADD_DATA_READER_FILE2,
  
  /** Create a data reader stream for incoming data using TCP
   * - uint64_t: stream number
   * - uint64_t: ip_addresses
   * - uint64_t: port
   **/
  MPI_TAG_ADD_DATA_READER_TCP2,
  
  /** Add a data writer to a file
   * - int32_t: channel number
   * - char[]: filename
   **/
  MPI_TAG_ADD_DATA_WRITER_FILE2,
  
  /** Add a void data writer
   * - int32_t: channel number
   **/
  MPI_TAG_ADD_DATA_WRITER_VOID2,
  

  /** This message is sent to the sending node, which creates the connection to the
   * receiving node, message contains the number of the MPI-node
   * (many -> one, e.g.\ input node -> correlator node)
   * - int32_t: stream nr for the data_writer
   * - int32_t: reader_rank
   * - int32_t: reader_stream_nr
   **/
  MPI_TAG_ADD_TCP,

  /** Acknowledge that an input connection is set up properly (for synchronisation)
   * - int32_t: Rank of the stream for the data reader
   **/
  MPI_TAG_CONNECTION_ESTABLISHED,
  
  // Node specific commands 
  //-------------------------------------------------------------------------//
  /** Terminate a node. Note that the Output node has its own message
   * - int32_t: no specific value
   **/
  MPI_TAG_END_NODE,
  
  // Input node specific commands
  //-------------------------------------------------------------------------//

  /** Goto the specified time in the stream.
   * - int64_t: Time in microseconds
   **/
  MPI_TAG_INPUT_NODE_GOTO_TIME,

  /** Returns a message with the same tag back with the current time stamp in 
   *  the channel extractor
   * - int64_t: the timestamp in microseconds
   **/
  MPI_TAG_INPUT_NODE_GET_CURRENT_TIMESTAMP,
   
  /** The stop time for the input node. Terminate if this time is reached
   * - int64_t: Time in microseconds
   **/
  MPI_TAG_INPUT_NODE_STOP_TIME,

  /** Adds a new writer to a time slicer
   * - int32_t: channel
   * - int32_t: stream
   * - int32_t: start time (milliseconds)
   * - int32_t: stop time (milliseconds)
   **/
  MPI_TAG_INPUT_NODE_ADD_TIME_SLICE,
   
  // Output node specific commands
  //-------------------------------------------------------------------------//

  /** Set the priority of the output stream
   * - int64_t: StreamNr
   * - int64_t: Priority
   * - int64_t: Size in bytes of the stream
   **/
  MPI_TAG_OUTPUT_STREAM_SLICE_SET_PRIORITY,
  
  // Correlate node specific commands
  //-------------------------------------------------------------------------//

  /** Send the Track parameters defined in Control_parameters.h
   * - ?
   **/
  MPI_TAG_TRACK_PARAMETERS,
  
  /** Send the Correlation parameters defined in Control_parameters.h
   * - ?
   **/
  MPI_TAG_CORR_PARAMETERS,
  
  /** Send a delay table
   * - ?
   **/
  MPI_TAG_DELAY_TABLE,
  
  /** The correlation node is ready to process data
   * - ?
   **/
  MPI_TAG_CORRELATION_OF_TIME_SLICE_ENDED,

  /** The output can terminate if a number of time slices is received
   * - int32_t: total number of time slices
   **/
  MPI_TAG_OUTPUT_NODE_CORRELATION_READY,

  /** The output node sends this message to the manager node if it is finished.
   * - int32_t: -
   **/
  MPI_TAG_OUTPUT_NODE_FINISHED,

  // Log node specific commands
  //-------------------------------------------------------------------------//

  /** Print all received log messages to std::cout
   * - ?
   **/
  MPI_TAG_LOG_NODE_SET_OUTPUT_COUT,
  
  /** Print all received log messages to file
   * - ?
   **/
  MPI_TAG_LOG_NODE_SET_OUTPUT_FILE,

  // General messages
  //-------------------------------------------------------------------------//

  /** The data at a buffer stopped
   * - ?
   **/
  MPI_TAG_DATASTREAM_EMPTY,
  
  /** A correlate node finished
   * - ?
   **/
  MPI_TAG_CORRELATE_ENDED,
  
  /** A log node terminated and will not send more messages:
   * - ?
   **/
  MPI_TAG_LOG_MESSAGES_ENDED,

  /** Message sent to the log node (LOG_NODE,
   * - ?
   **/
  MPI_TAG_TEXT_MESSAGE,

  /** Send a log message to the manager node
   * - ?
   **/
  MPI_TAG_LOG_MESSAGE,

  MPI_TAG_ERROR
};

// Helps detecting missing constants in MPI_TAG:
// generate with: 
// sed -e "s://.*::" -e "s:[ ,(=0)]::g" -e "/^$/d"
//     -e "s:^\(.*\)$:    case \1\:\n      \{ return \"\1\"; \}:"
inline const char * const do_print_MPI_TAG(MPI_TAG tag) {
  switch (tag) {
    case MPI_TAG_ADD_DATA_WRITER_FILE2:
      { return "MPI_TAG_ADD_DATA_WRITER_FILE"; }
    case MPI_TAG_ADD_DATA_WRITER_VOID2:
      { return "MPI_TAG_ADD_DATA_WRITER_VOID"; }
    case MPI_TAG_ADD_TCP:
      { return "MPI_TAG_ADD_TCP"; }
    case MPI_TAG_CONNECTION_ESTABLISHED:
      { return "MPI_TAG_CONNECTION_ESTABLISHED"; }
    case MPI_TAG_SET_INPUT_NODE:
      { return "MPI_TAG_SET_INPUT_NODE"; }
    case MPI_TAG_SET_CORRELATOR_NODE:
      { return "MPI_TAG_SET_CORRELATOR_NODE"; }
    case MPI_TAG_SET_OUTPUT_NODE:
      { return "MPI_TAG_SET_OUTPUT_NODE"; }
    case MPI_TAG_SET_LOG_NODE:
      { return "MPI_TAG_SET_LOG_NODE"; }
    case MPI_TAG_NODE_INITIALISED:
      { return "MPI_TAG_NODE_INITIALISED"; }
    case MPI_TAG_GET_STATUS:
      { return "MPI_TAG_GET_STATUS"; }
    case MPI_TAG_SET_MESSAGELEVEL:
      { return "MPI_TAG_SET_MESSAGELEVEL"; }
    case MPI_TAG_ADD_DATA_READER_FILE2:
      { return "MPI_TAG_ADD_DATA_READER_FILE"; }
    case MPI_TAG_ADD_DATA_READER_TCP2:
      { return "MPI_TAG_ADD_DATA_READER_TCP"; }
    case MPI_TAG_INPUT_NODE_GOTO_TIME:
      { return "MPI_TAG_INPUT_NODE_GOTO_TIME"; }
    case MPI_TAG_INPUT_NODE_GET_CURRENT_TIMESTAMP:
      { return "MPI_TAG_GET_CURRENT_TIMESTAMP"; }
    case MPI_TAG_INPUT_NODE_STOP_TIME:
      { return "MPI_TAG_INPUT_NODE_STOP_TIME"; }
    case MPI_TAG_INPUT_NODE_ADD_TIME_SLICE:
      { return "MPI_TAG_INPUT_NODE_ADD_TIME_SLICE"; }
    case MPI_TAG_OUTPUT_STREAM_SLICE_SET_PRIORITY:
      { return "MPI_TAG_OUTPUT_STREAM_SLICE_SET_PRIORITY"; }
    case MPI_TAG_TRACK_PARAMETERS:
      { return "MPI_TAG_TRACK_PARAMETERS"; }
    case   MPI_TAG_CORR_PARAMETERS:
      { return "MPI_TAG_CORR_PARAMETERS"; }
    case MPI_TAG_DELAY_TABLE:
      { return "MPI_TAG_DELAY_TABLE"; }
    case MPI_TAG_CORRELATION_OF_TIME_SLICE_ENDED:
      { return "MPI_TAG_CORRELATION_OF_TIME_SLICE_ENDED"; }
    case MPI_TAG_END_NODE:
      { return "MPI_TAG_END_NODE"; }
    case MPI_TAG_OUTPUT_NODE_CORRELATION_READY:
      { return "MPI_TAG_OUTPUT_NODE_CORRELATION_READY"; }
    case MPI_TAG_OUTPUT_NODE_FINISHED:
      { return "MPI_TAG_OUTPUT_NODE_FINISHED"; }
    case MPI_TAG_LOG_NODE_SET_OUTPUT_COUT:
      { return "MPI_TAG_LOG_NODE_SET_OUTPUT_COUT"; }
    case MPI_TAG_LOG_NODE_SET_OUTPUT_FILE:
      { return "MPI_TAG_LOG_NODE_SET_OUTPUT_FILE"; }
    case MPI_TAG_DATASTREAM_EMPTY:
      { return "MPI_TAG_DATASTREAM_EMPTY"; }
    case MPI_TAG_CORRELATE_ENDED:
      { return "MPI_TAG_CORRELATE_ENDED"; }
    case MPI_TAG_LOG_MESSAGES_ENDED:
      { return "MPI_TAG_LOG_MESSAGES_ENDED"; }
    case MPI_TAG_TEXT_MESSAGE:
      { return "MPI_TAG_TEXT_MESSAGE"; }
    case MPI_TAG_LOG_MESSAGE:
      { return "MPI_TAG_LOG_MESSAGE"; }
    case MPI_TAG_ERROR:
      { return "MPI_TAG_ERROR"; }
  }
  return "UNKNOWN_MPI_TAG";
}

inline const char * const print_MPI_TAG(int tag) {
  return do_print_MPI_TAG(static_cast<MPI_TAG>(tag));
}
inline const char * const print_MPI_TAG(MPI_TAG &tag) {
  return do_print_MPI_TAG(tag);
}


//undef have to be before include <mpi.h>
#ifdef SEEK_SET
#undef SEEK_SET
#endif
#ifdef SEEK_END
#undef SEEK_END
#endif
#ifdef SEEK_CUR
#undef SEEK_CUR
#endif

// disable mpi warnings:
#pragma GCC system_header
#include <mpi.h>

#endif /*SFXC_MPI_H*/