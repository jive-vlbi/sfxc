/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef OUTPUT_NODE_H
#define OUTPUT_NODE_H

#include <Node.h>
#include <Multiple_data_readers_controller.h>
#include <Single_data_writer_controller.h>

#include <map>
#include <queue>

class Output_node;

/// Controller for output node specific commands
class Output_node_controller : public Controller {
public:
  typedef std::map<UINT32, int>                 Input_stream_priority_map;
  typedef Input_stream_priority_map::value_type Input_stream_priority_map_value;

  Output_node_controller(Output_node &node);
  
  Process_event_status process_event(MPI_Status &status);
  
private:
  Output_node &node;
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
class Output_node : public Node {
public:
  typedef Buffer_element<char,131072>           value_type;
  typedef std::map<INT32, int>                  Input_stream_priority_map;
  typedef Input_stream_priority_map::value_type Input_stream_priority_map_value;
  typedef Buffer<value_type>                    Buffer;
  
  /** Manages the input from one correlator node.
   * The input stream is used to maintain the data 
   **/
  class Input_stream {
  public:
    Input_stream(Data_reader *reader);
    
    /** Fills the buffer with as much data as possible and returns the number of
     * bytes written.
     **/
    int write_bytes(value_type &elem);
    /** returns whether we reached the end of the current time slice
     **/
    bool end_of_slice();
    /** returns whether there is data available
     **/
    bool has_data();
    
    /** sets the length of a new time slice
     **/
    void set_length_time_slice(UINT64 nBytes);
  private:
    // Data_reader from which the input data can be read
    Data_reader *reader;
    // The number of bytes left in the current slice
    int curr_bytes;
    // list with sizes of the time slices
    std::queue<UINT64> slice_size;
  };

  Output_node(int rank, Log_writer *writer, int buffer_size = 10);
  Output_node(int rank, int buffer_size = 10);
  void initialise();

  ~Output_node();

  void start();

  enum STATUS {
    STOPPED=0,
    WRITE_OUTPUT,
    END_NODE
  };
  
  void set_status();

  void create_buffer(int num);

  void set_weight_of_input_stream(int num, UINT64 weight);
  void time_slice_finished(int rank, UINT64 nBytes);      
  
  void set_number_of_time_slices(int n_time_slices);

  // Callback functions:
  void hook_added_data_reader(int reader);
  void hook_added_data_writer(int writer);

private:
  bool data_available();
  void write_output();


  // Output buffer:
  Semaphore_buffer<value_type>        output_buffer;

  // Controllers:
  Output_node_controller              output_node_ctrl;
  Multiple_data_readers_controller    data_readers_ctrl;
  Single_data_writer_controller       data_writer_ctrl;

  STATUS                              status;
  // Priority map of the input streams
  Input_stream_priority_map           input_streams_order;
  // One input stream for every correlate node
  std::vector<Input_stream *>         input_streams;
  
  /** List of streams that stores whether a time slice is transferred.
   * The stream is not finished, if input_streams_finished[i]==0
   * if input_streams_finished[i]>0 then the stream is finished and specifies
   * the number of bytes belonging to the slice
   **/
  INT32 curr_slice,number_of_time_slices;
//  std::vector<UINT64>                 input_streams_finished;
  /// the data_readers read the data from the buffers in the data_readers_ctrl.
//  std::vector<Data_reader *>          data_readers;
};

#endif // OUTPUT_NODE_H
