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

#include "node.h"
#include "multiple_data_readers_controller.h"
#include "single_data_writer_controller.h"
#include "output_header.h"

#include <memory_pool.h>

#include <map>
#include <queue>

class Output_node;

/// Controller for output node specific commands
class Output_node_controller : public Controller {
public:
  typedef std::map<uint32_t, int>                 Input_stream_priority_map;
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
  typedef Multiple_data_readers_controller::value_type input_value_type;
  typedef std::map<int32_t, int>                    Input_stream_priority_map;
  typedef Input_stream_priority_map::value_type     Input_stream_priority_map_value;
  typedef Buffer<input_value_type>                  Input_buffer;

  typedef Single_data_writer_controller::value_type  output_value_type;
  typedef Single_data_writer_controller::Memory_pool Output_memory_pool;
  typedef Single_data_writer_controller::Queue       Output_queue;
  typedef Single_data_writer_controller::Queue_ptr   Output_queue_ptr;

  /** Manages the input from one correlator node.
   * The input stream is used to maintain the data
   **/
  class Input_stream {
  public:
    Input_stream(boost::shared_ptr<Data_reader> reader);

    /** Fills the buffer with as much data as possible and returns the number of
     * bytes written.
     **/
    void write_bytes(output_value_type &elem);
    /** returns whether we reached the end of the current time slice
     **/
    bool end_of_slice();

    /** sets the length of a new time slice
     **/
    void set_length_time_slice(int64_t nBytes);

    /** Goto the next data slice **/
    void goto_next_slice();
  private:
    // Data_reader from which the input data can be read
    boost::shared_ptr<Data_reader> reader;
    // list with sizes of the time slices
    std::queue<int64_t> slice_size;
  };

  Output_node(int rank, Log_writer *writer, int buffer_size = 10);
  Output_node(int rank, int buffer_size = 10);
  void initialise();

  ~Output_node();

  void start();

  enum STATUS {
    STOPPED=0,
    START_NEW_SLICE,
    WRITE_OUTPUT,
    END_SLICE,
    END_NODE
  };

  void write_global_header(const Output_header_global &global_header);
  void set_weight_of_input_stream(int num, int64_t weight, size_t size);
  void time_slice_finished(int rank, int64_t nBytes);

  void set_number_of_time_slices(int n_time_slices);

  // Callback functions:
  void hook_added_data_reader(size_t reader);
  void hook_added_data_writer(size_t writer);

private:
  void write_output();


  // Output buffer:
  Output_memory_pool                  output_memory_pool;
  Output_queue_ptr                    output_queue;

  // Controllers:
  Output_node_controller              output_node_ctrl;
  Multiple_data_readers_controller    data_readers_ctrl;
  Single_data_writer_controller       data_writer_ctrl;

  STATUS                              status;
  // Priority map of the input streams
  Input_stream_priority_map           input_streams_order;
  // One input stream for every correlate node
  std::vector<Input_stream *>         input_streams;

  int32_t curr_slice, number_of_time_slices, curr_stream;
};

#endif // OUTPUT_NODE_H
