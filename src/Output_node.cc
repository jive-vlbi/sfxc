/* Author(s): Nico Kruithof, 2007
 * 
 * $Id$
 */

#include <Output_node.h>

#include <types.h>
#include <Data_writer_file.h>
#include <Queue_buffer.h>

#include <iostream>
#include <assert.h>

Output_node::Output_node(int rank, int size)
  : Node(rank),
    output_node_ctrl(*this),
    data_readers_ctrl(get_log_writer()),
    data_writer_ctrl(get_log_writer()),
    status(STOPPED)
{
  initialise();
}

Output_node::Output_node(int rank, Log_writer *writer, int size) 
  : Node(rank, writer),
    output_node_ctrl(*this),
    data_readers_ctrl(get_log_writer()),
    data_writer_ctrl(get_log_writer())
{
  initialise(); 
}

void Output_node::initialise() {
  add_controller(&data_readers_ctrl);
  add_controller(&output_node_ctrl);
  add_controller(&data_writer_ctrl);
  
  INT32 msg;
  MPI_Send(&msg, 1, MPI_INT32, 
           RANK_MANAGER_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD);
}

Output_node::~Output_node() {
  get_log_writer()(1) << "~Output_node()";
}

void Output_node::start() {
  while (status != END_NODE) {
    switch (status) {
      case STOPPED: {
        // blocking:
        if (check_and_process_message()==TERMINATE_NODE) {
          status = END_NODE;
        }
        break;
      }
      case WRITE_OUTPUT: {
        while ((check_and_process_waiting_message() != NO_MESSAGE) &&
               (status != END_NODE)) {
        }
        
        write_output();
        break;
      }
      case END_NODE: { // For completeness sake, is caught by the while loop
        break;
      }
    }
  }
}

void Output_node::set_weight_of_input_stream(int num, UINT64 weight) {
  assert(num >= 0);
  
  // Create an output buffer:
  assert(data_readers_ctrl.get_buffer(num) == NULL);
  Buffer *new_buffer = new Queue_buffer<value_type>();
  data_readers_ctrl.set_buffer(num, new_buffer);

  // Add the weight to the priority queue:
  // Check that the weight does not exist yet and is larger than the minimum:
  assert(input_streams_order.find(weight) == input_streams_order.end());
  if (!input_streams_order.empty()) {
    assert(input_streams_order.begin()->first < weight);
  }
  input_streams_order.insert(Input_stream_priority_map_value(weight,num));
  
  // Set finished to false:
  if (input_streams_finished.size() <= (unsigned int)num) {
    input_streams_finished.resize(num+1);
  }
  input_streams_finished[num] = false;
  status = WRITE_OUTPUT;
}

void Output_node::time_slice_finished(int num) {
  assert(input_streams_finished.size() > (unsigned int) num);
  assert(!input_streams_finished[num]);
  input_streams_finished[num] = true;
}

void Output_node::set_status() {
  status = STOPPED;
  if (!input_streams_order.empty()) status = WRITE_OUTPUT;
}

void Output_node::write_output() {
  int head = input_streams_order.begin()->second;
  Buffer *buffer = data_readers_ctrl.get_buffer(head);
  if (buffer->empty()) {
    if (input_streams_finished[head]) {
      input_streams_order.erase(input_streams_order.begin());
      input_streams_finished[head] = false;
    } else {
      usleep(100000); // .1 second:
    }
  } else {
    std::cout << "WRITE_OUTPUT" << std::endl;
    int status;
    value_type &in_elem = buffer->consume(status);
    value_type &out_elem = data_writer_ctrl.buffer()->produce();
    memcpy(&out_elem, &in_elem, status); 
    data_writer_ctrl.buffer()->produced(status);
    buffer->consumed();
  }
}
