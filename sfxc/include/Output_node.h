/* Author(s): Nico Kruithof, 2007
 * 
 * $Id$
 */

#ifndef OUTPUT_NODE_H
#define OUTPUT_NODE_H

#include <Node.h>
#include <Multiple_data_readers_controller.h>
#include <Single_data_writer_controller.h>

#include <map>

class Output_node;

/// Controller for output node specific commands
class Output_node_controller : public Controller {
public:
  typedef std::map<UINT64, int>                 Input_stream_priority_map;
  typedef Input_stream_priority_map::value_type Input_stream_priority_map_value;

  Output_node_controller(Output_node &node);
  
  Process_event_status process_event(MPI_Status &status);
  
private:
  Output_node &node;
};

/**
 * \ingroup Node
 **/
class Output_node : public Node {
public:
  typedef Buffer_element<char,131072>           value_type;
  typedef std::map<UINT64, int>                 Input_stream_priority_map;
  typedef Input_stream_priority_map::value_type Input_stream_priority_map_value;
  typedef Buffer<value_type>                    Buffer;

  Output_node(int rank, Log_writer *writer, int buffer_size = 1024);
  Output_node(int rank, int buffer_size = 1024);
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
  void time_slice_finished(int num);      
  
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

  STATUS                                       status;
  Input_stream_priority_map                    input_streams_order;
  std::vector<bool>                            input_streams_finished;
};

#endif // OUTPUT_NODE_H
