/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: Output_node_without_buffering.h 222 2007-05-14 07:22:32Z kruithof $
 *
 */

#ifndef OUTPUT_NODE_WITHOUT_BUFFERING_H
#define OUTPUT_NODE_WITHOUT_BUFFERING_H

#include <Node.h>
#include <Multiple_data_readers_controller.h>
#include <Single_data_writer_controller.h>

#include <map>
#include <queue>

class Output_node_without_buffering;

/// Controller for output node specific commands
class Output_node_without_buffering_controller : public Controller {
public:
  typedef std::map<uint32_t, int>                 Input_stream_priority_map;
  typedef Input_stream_priority_map::value_type Input_stream_priority_map_value;

  Output_node_without_buffering_controller(Output_node_without_buffering &node);
  
  Process_event_status process_event(MPI_Status &status);
  
private:
  Output_node_without_buffering &node;
};

/**
 * The output node will receive a message from the controller node where
 * to store the data and it allows connections from the correlate node to
 * be opened. The node sorts the received data from the correlate nodes
 * and stores it for further processing. The output node has to make the
 * received data available to the user and it should be archived in a
 * proper way.
 * 
 * \ingroup Node
 **/
class Output_node_without_buffering : public Node {
public:
  typedef Buffer_element<char,131072>           value_type;
  typedef std::map<int32_t, int>                  Input_stream_priority_map;
  typedef Input_stream_priority_map::value_type Input_stream_priority_map_value;
  typedef Buffer<value_type>                    Buffer;
  
  Output_node_without_buffering(int rank, Log_writer *writer, int buffer_size = 10);
  Output_node_without_buffering(int rank, int buffer_size = 10);
  void initialise();

  ~Output_node_without_buffering();

  void start();

  enum STATUS {
    STOPPED=0,
    WRITE_OUTPUT,
    END_NODE
  };
  
  void set_status();

  void create_buffer(int num);

  void set_weight_of_input_stream(int num, uint64_t weight);
  void time_slice_finished(int rank, uint64_t nBytes);      
  
  void set_number_of_time_slices(int n_time_slices);

  // Callback functions:
  void hook_added_data_reader(size_t reader);
  void hook_added_data_writer(size_t writer);

private:
  bool data_available();
  void write_output();


  // Output buffer:
  Semaphore_buffer<value_type>        output_buffer;

  // Controllers:
  Output_node_without_buffering_controller              Output_node_without_buffering_ctrl;
  Multiple_data_readers_controller    data_readers_ctrl;
  Single_data_writer_controller       data_writer_ctrl;

  STATUS                              status;
  // Priority map of the input streams
  Input_stream_priority_map           input_streams_order;
  std::vector<int>                    bytes_in_timeslice_per_input_stream;
  
  /** List of streams that stores whether a time slice is transferred.
   * The stream is not finished, if input_streams_finished[i]==0
   * if input_streams_finished[i]>0 then the stream is finished and specifies
   * the number of bytes belonging to the slice
   **/
  int32_t curr_slice,number_of_time_slices;
};

#endif // OUTPUT_NODE_WITHOUT_BUFFERING_H
