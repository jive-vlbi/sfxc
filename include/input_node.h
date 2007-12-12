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

#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "node.h"
#include "single_data_reader_controller.h"
#include "multiple_data_writers_controller.h"

#include "channel_extractor_mark4.h"
#include "time_slicer.h"

#include "data_reader2buffer.h"

#include "semaphore_buffer.h"


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
  typedef Semaphore_buffer<value_type>                  Buffer;

  // assume at most 8 tracks:
  typedef Time_slicer< Buffer_element_large<char, SIZE_MK4_FRAME> >    Slicer;
public:
  Input_node(int rank, int station_number, Log_writer *log_writer);
  Input_node(int rank, int station_number);
  ~Input_node();
  
  /** Generic constructor function, that is called in the body of
      every constructor.
  **/
  void initialise();

  /** Sets the track parameters **/
  void set_track_parameters(const Track_parameters &track_param);

  
  /// Start the state machine
  void start();

  /// Status of the state machine
  enum Status {
    WAITING=0,    ///< The input node is waiting
    INITIALISING, ///< Waiting for all channels to get connected
    WRITING,      ///< Writing the output of the current channel
    END_NODE      ///< Terminate the node
  };
  
  /// Get the current time stamp  
  int64_t get_time_stamp();
  
  void set_stop_time(int64_t stop_time);

  void goto_time(int64_t new_time);

  void add_time_slice(int channel, int stream, int starttime, int stoptime);

  int get_status();
  
  // Callback functions:
  void hook_added_data_reader(size_t reader);
  void hook_added_data_writer(size_t writer);

private:

  /// Controller for the input node (messages specific for the input node).
  Input_node_controller                        input_node_ctrl;
  /// An Input_node has one data stream coming in.
  Single_data_reader_controller                data_reader_ctrl;
  /// An Input_node has several data streams for output.
  Multiple_data_writers_controller             data_writers_ctrl;

  /// The channel extractor
  boost::shared_ptr<Channel_extractor_mark4> channel_extractor;
  /// A list of time slicers, one per channel
  std::vector< Slicer >                      time_slicers;
  
  Status status;

  int32_t start_time;
  
  int64_t stop_time;
};

#endif // INPUT_NODE_H