/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef INPUT_NODE_H
#define INPUT_NODE_H

#include <Node.h>
#include <Single_data_reader_controller.h>
#include <Multiple_data_writers_controller.h>

#include <Channel_extractor.h>

#include <Data_reader2buffer.h>

#include <Semaphore_buffer.h>

#include <map>
#include <vector>

#include <constPrms.h>
#include <runPrms.h>
#include <genPrms.h>
#include <staPrms.h>

extern RunP  RunPrms;
extern GenP  GenPrms;
extern StaP  StaPrms[NstationsMax];



class Input_node;

/// Controller for input node specific commands
class Input_node_controller : public Controller {
public:
  Input_node_controller(Input_node &node);
  
  Process_event_status process_event(MPI_Status &status);
  
private:
  Input_node &node;
};

/**
 * The input node opens a controller for reading data and one for
 * forwarding the data. It then connects the two using a buffer. The data
 * node will receive a message from the controller node specifying how to
 * obtain the input: from file or over the network using one of various
 * types of transfer protocols. It will also receive messages containing
 * a start and stop time and the correlate node to send the data to.
 * 
 * \ingroup Node
 **/
class Input_node : public Node {
  typedef Input_node                       Self;
  
  typedef Single_data_reader_controller::value_type     value_type;
  typedef Semaphore_buffer<value_type>     Buffer;
public:
  Input_node(int rank, Log_writer *log_writer);
  Input_node(int rank);
  ~Input_node();
  
  /// Generic constructor function, that is called in the body of every constructor
  void initialise();
  
  /// Start the state machine
  void start();

  /// Status of the state machine
  enum STATUS {
    STOPPED=0, ///< The input node is waiting
    SEND_OUTPUT, ///< The input node is forwarding data
    END_NODE ///< The input node is shutting down
  };
  
  /// Automatically determine the status of the state machine
  void set_status();
  
  /// Set the start and stop time of an output stream 
  void set_priority(int stream, UINT64 start, UINT64 stop);

  /// Update the current time stamp
  void set_time_stamp(value_type &t);
  
  /// Get the current time stamp  
  UINT64 get_time_stamp() { return time_stamp; }
  
  /// Check whether we need to start or stop output streams:  
  void update_active_list();
  /// Add an output stream to the list of active output streams
  void add_to_active_list(int stream);
  /// Remove an output stream from the list of active output streams
  void remove_from_active_list(int stream);

  // Callback functions:
  void hook_added_data_reader(size_t reader);
  void hook_added_data_writer(size_t writer);

private:
  // Stop a stream in active_list
  void stop_stream(const std::list<int>::iterator &stream_it);

  /// Controller for the input node (messages specific for the input node).
  Input_node_controller                        input_node_ctrl;
  /// An Input_node has one data stream coming in.
  Single_data_reader_controller                data_reader_ctrl;
  /// An Input_node has several data streams for output.
  Multiple_data_writers_controller             data_writers_ctrl;

  Channel_extractor * channel_extractor;

  
  /// Two queues for starting and stopping of output streams
  std::multimap<UINT64, int>                   start_queue, stop_queue;
  /// The input stream is redirected to the streams in the active list: 
  std::list<int>                               active_list;
  /// Current timestamp, used for starting and stopping output streams:
  UINT64                                       time_stamp;
  
  /// Number of elements in a buffer
  int                                          buffer_size;
  
  /// Number of the input reader (input readers should be numbered 0..N)
  int nr_input_reader;
  int get_input_node_number() {
    return nr_input_reader;
  }
  /// Status of the state machine
  STATUS                                       status;


  static const size_t ch_buffer_size = 131072;
  char ch_buffer[ch_buffer_size];
  
};

#endif // INPUT_NODE_H
