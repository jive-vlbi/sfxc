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

  // DATA COMMUNICATION, SET is for a single connection, ADD for multiple
  //--------------------------------------------------------------------------

  /** Create a data reader from a file
   * - ?
   **/
  MPI_TAG_SET_DATA_READER_FILE,
  /** Create a data reader from a TCP connection
   * - ?
   **/
  MPI_TAG_SET_DATA_READER_TCP,
  /** Create a data writer to a file
   * - ?
   **/
  MPI_TAG_SET_DATA_WRITER_FILE,
  /** Create a data writer to a TCP connection, the input is 
   * - UINT64+: ip_addresses
   * - UINT64 port
   **/
  MPI_TAG_SET_DATA_WRITER_TCP,

  /** Create a data reader stream for incoming data using TCP
   * - CHAR: stream number
   * - CHAR+: filename
   **/
  MPI_TAG_ADD_DATA_READER_FILE,
  /** Create a data reader stream for incoming data using TCP
   * - UINT64: ip_addresses
   * - UINT64: port
   **/
  MPI_TAG_ADD_DATA_READER_TCP,
  /** Add a data writer to a file
   * - ?
   **/
  MPI_TAG_ADD_DATA_WRITER_FILE,
  /** Set the output stream for a correlate node
   * - UINT64+ ip_addresses,
   * - UINT64 port
   **/
  MPI_TAG_ADD_DATA_WRITER_TCP,

  /** This message is sent to the sending node, which creates the connection to the
   * receiving node, message contains the number of the MPI-node
   * (many -> one, e.g.\ input node -> correlator node)
   * - ?
   **/
  MPI_TAG_ADD_OUTPUT_CONNECTION_SINGLE_INPUT_TCP,
  /** This message is sent to the sending node, which creates the connection to the
   * receiving node, message contains the number of the MPI-node (many -> many)
   * - INT32: Rank of the stream for the data reader
   * - INT32: Rank of the stream for the data writer
   **/
  MPI_TAG_ADD_OUTPUT_CONNECTION_MULTIPLE_INPUT_TCP,

  /** This message sets up the communication between two nodes using MPI 
   * - INT32: stream number for the data writer
   * - INT32: stream number for the data reader
   * - INT32: rank of the data_reader
   **/   
  MPI_TAG_ADD_OUTPUT_CONNECTION_MULTIPLE_INPUT_MPI,
  
  /** Acknowledge that an input connection is set up properly (for synchronisation)
   * - INT32: Rank of the stream for the data reader
   * - INT32: Rank of the stream for the data writer
   * - INT32: Rank of the node to connect to
   **/
  MPI_TAG_INPUT_CONNECTION_ESTABLISHED,
  
  /** This message is sent to the sending node, which creates the connection to
   * the receiving node, message contains the number of the MPI-node 
   * (one -> one)
   * - ?
   **/
  MPI_TAG_SET_OUTPUT_CONNECTION_SINGLE_INPUT_TCP,
  /** one -> many (correlator node -> log node / output node)
   * - ?
   **/
  MPI_TAG_SET_OUTPUT_CONNECTION_MULTIPLE_INPUT_TCP,
  
  // Node specific commands 
  //-------------------------------------------------------------------------//
  /** Terminate a node. Note that the Output node has its own message
   * - INT32: no specific value
   **/
  MPI_TAG_CORRELATION_READY,
  
  // Input node specific commands
  //-------------------------------------------------------------------------//

  /** Set the priority of the input stream
   * Three values of INT64: {StreamNr, StartTag, StopTag}
   * - INT64: StreamNr
   * - INT64: StartTag
   * - INT64: StopTag
   **/
  MPI_TAG_INPUT_STREAM_SET_PRIORITY,

  // Output node specific commands
  //-------------------------------------------------------------------------//

  /** Set the priority of the output stream
   * - INT64: StreamNr
   * - INT64: Priority
   **/
  MPI_TAG_OUTPUT_STREAM_SET_PRIORITY,
  
  // Correlate node specific commands
  //-------------------------------------------------------------------------//

  /** Send the control parameters for a correlator node
   * - ?
   **/
  MPI_TAG_CONTROL_PARAM,
  /** Send a delay table
   * - ?
   **/
  MPI_TAG_DELAY_TABLE,
  /** All settings are set for the correlation, start correlating
   * Send the slice number, start time and duration to a correlate_node
   * - ?
   **/
  MPI_TAG_CORRELATE_TIME_SLICE,
  /** The output stream from the sending node is finished for this time slice
   * - UINT64 input stream
   * - UINT64 nBytes
   **/
  MPI_TAG_OUTPUT_STREAM_TIME_SLICE_FINISHED,
  /** The correlation node is ready to process data
   * - ?
   **/
  MPI_TAG_CORRELATION_OF_TIME_SLICE_ENDED,

  /** The output can terminate if a number of time slices is received
   * - INT32: total number of time slices
   **/
  MPI_TAG_OUTPUT_NODE_CORRELATION_READY,

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
    case MPI_TAG_ADD_DATA_WRITER_FILE:
      { return "MPI_TAG_ADD_DATA_WRITER_FILE"; }
    case MPI_TAG_ADD_OUTPUT_CONNECTION_SINGLE_INPUT_TCP:
      { return "MPI_TAG_ADD_OUTPUT_CONNECTION_SINGLE_INPUT_TCP"; }
    case MPI_TAG_ADD_OUTPUT_CONNECTION_MULTIPLE_INPUT_TCP:
      { return "MPI_TAG_ADD_OUTPUT_CONNECTION_MULTIPLE_INPUT_TCP"; }
    case MPI_TAG_ADD_OUTPUT_CONNECTION_MULTIPLE_INPUT_MPI:
      { return "MPI_TAG_ADD_OUTPUT_CONNECTION_MULTIPLE_INPUT_MPI"; }
    case MPI_TAG_SET_OUTPUT_CONNECTION_SINGLE_INPUT_TCP:
      { return "MPI_TAG_SET_OUTPUT_CONNECTION_SINGLE_INPUT_TCP"; }
    case MPI_TAG_SET_OUTPUT_CONNECTION_MULTIPLE_INPUT_TCP:
      { return "MPI_TAG_SET_OUTPUT_CONNECTION_MULTIPLE_INPUT_TCP"; }
    case MPI_TAG_INPUT_CONNECTION_ESTABLISHED:
      { return "MPI_TAG_INPUT_CONNECTION_ESTABLISHED"; }
    case MPI_TAG_SET_DATA_WRITER_FILE:
      { return "MPI_TAG_SET_DATA_WRITER_FILE"; }
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
    case MPI_TAG_SET_DATA_READER_FILE:
      { return "MPI_TAG_SET_DATA_READER_FILE"; }
    case MPI_TAG_SET_DATA_READER_TCP:
      { return "MPI_TAG_SET_DATA_READER_TCP"; }
    case MPI_TAG_SET_DATA_WRITER_TCP:
      { return "MPI_TAG_SET_DATA_WRITER_TCP"; }
    case MPI_TAG_ADD_DATA_READER_FILE:
      { return "MPI_TAG_ADD_DATA_READER_FILE"; }
    case MPI_TAG_ADD_DATA_READER_TCP:
      { return "MPI_TAG_ADD_DATA_READER_TCP"; }
    case MPI_TAG_ADD_DATA_WRITER_TCP:
      { return "MPI_TAG_ADD_DATA_WRITER_TCP"; }
    case MPI_TAG_INPUT_STREAM_SET_PRIORITY:
      { return "MPI_TAG_INPUT_STREAM_SET_PRIORITY"; }
    case MPI_TAG_OUTPUT_STREAM_SET_PRIORITY:
      { return "MPI_TAG_OUTPUT_STREAM_SET_PRIORITY"; }
    case MPI_TAG_CONTROL_PARAM:
      { return "MPI_TAG_CONTROL_PARAM"; }
    case MPI_TAG_DELAY_TABLE:
      { return "MPI_TAG_DELAY_TABLE"; }
    case MPI_TAG_CORRELATE_TIME_SLICE:
      { return "MPI_TAG_CORRELATE_TIME_SLICE"; }
    case MPI_TAG_OUTPUT_STREAM_TIME_SLICE_FINISHED:
      { return "MPI_TAG_OUTPUT_STREAM_TIME_SLICE_FINISHED"; }
    case MPI_TAG_CORRELATION_OF_TIME_SLICE_ENDED:
      { return "MPI_TAG_CORRELATION_OF_TIME_SLICE_ENDED"; }
    case MPI_TAG_CORRELATION_READY:
      { return "MPI_TAG_CORRELATION_READY"; }
    case MPI_TAG_OUTPUT_NODE_CORRELATION_READY:
      { return "MPI_TAG_OUTPUT_NODE_CORRELATION_READY"; }
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
