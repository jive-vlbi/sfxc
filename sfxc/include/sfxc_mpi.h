#ifndef SFXC_MPI_H
#define SFXC_MPI_H

#define LOG_NODE 0

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
  /// Make an output node write data to file
  MPI_TAG_SET_OUTPUT_NODE_FILE,

  /// Create an output stream for a correlate node (input stream for the output node)
  MPI_TAG_CREATE_OUTPUT_STREAM_TCP,

  /** Set the output stream for a correlate node.
   * Data: UINT64 ... ip_addresses, UINT64 port
   **/
  MPI_TAG_SET_OUTPUT_STREAM_TCP,

  
  /// Set the weight of a output stream. 
  /// Output streams are written with increasing weights.
  MPI_TAG_SET_WEIGHT_OUTPUT_STREAM,
  
  /// The output stream from the sending node is finished for this time slice.
  MPI_TAG_OUTPUT_STREAM_TIME_SLICE_FINISHED,

  /// Set the station number for the commands following
  MPI_TAG_SET_STATION_NUMBER,
  /// Send the start time for a correlate_node
  MPI_TAG_SET_START_TIME,
  /// Send the stop time for a correlate_node
  MPI_TAG_SET_STOP_TIME,
  /// Send the start time for a correlate_node
  MPI_TAG_SET_TIME_SLICE,
  /// Send the control parameters for a correlator node
  MPI_TAG_CONTROL_PARAM,
  /// Send a delay table
  MPI_TAG_DELAY_TABLE,

  /// All settings are set for the correlation, start correlating
  MPI_TAG_START_CORRELATE_NODE,
  
  /// Set the delay table for a data stream in a correlate node (DEPRECATED)
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
  MPI_TAG_DATASTREAM_EMPTY,
  
  /// A correlate node finished
  MPI_TAG_CORRELATE_ENDED,

  /// The correlation node is ready to process data
  MPI_TAG_CORRELATION_READY,
  
  /// Message sent to the log node (LOG_NODE)
  MPI_TAG_TEXT_MESSAGE,
  
  /// Send a log message to the manager node
  MPI_TAG_LOG_MESSAGE,

  MPI_TAG_ERROR
};


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
