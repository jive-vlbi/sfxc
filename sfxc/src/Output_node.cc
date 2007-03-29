/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 */

#include <Output_node.h>

#include <types.h>
#include <Data_writer_file.h>
#include <Data_reader_buffer.h>
#include <Queue_buffer.h>

#include <iostream>
#include <assert.h>

Output_node::Output_node(int rank, int size)
  : Node(rank),
    output_buffer(1000),
    output_node_ctrl(*this),
    data_readers_ctrl(*this),
    data_writer_ctrl(*this),
    status(STOPPED)
{
  initialise();
}

Output_node::Output_node(int rank, Log_writer *writer, int size) 
  : Node(rank, writer),
    output_buffer(1000),
    output_node_ctrl(*this),
    data_readers_ctrl(*this),
    data_writer_ctrl(*this)
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
           
  data_writer_ctrl.set_buffer(&output_buffer);  
}

Output_node::~Output_node() {
  // empty the input buffers to the output

  while (data_available()) {
    // We might not have received all the time slice termination messages yet:
    while (check_and_process_waiting_message() != NO_MESSAGE) {}
    write_output();
  }
  
  // wait until the output buffer is empty
  while (!output_buffer.empty()) {
    usleep(100000);
  }
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
        if (process_all_waiting_messages() == TERMINATE_NODE) {
          status = END_NODE;
          break;
        }
        
        if (data_available()) {
          write_output();
        }
        break;
      }
      case END_NODE: { // For completeness sake
        break;
      }
    }
  }
}

void Output_node::create_buffer(int num) {
//  // Create an output buffer:
//  assert(data_readers_ctrl.get_buffer(num) == NULL);
//  Buffer *new_buffer = new Queue_buffer<value_type>();
//  data_readers_ctrl.set_buffer(num, new_buffer);
}

void Output_node::set_weight_of_input_stream(int num, UINT64 weight) {
  assert(num >= 0);
  
    // Add the weight to the priority queue:
  // Check that the weight does not exist yet and is larger than the minimum:
  assert(input_streams_order.find(weight) == input_streams_order.end());
  if (!input_streams_order.empty()) {
    assert(input_streams_order.begin()->first < weight);
  }
  input_streams_order.insert(Input_stream_priority_map_value(weight,num));
  
  status = WRITE_OUTPUT;
}

void Output_node::time_slice_finished(int rank, UINT64 nBytes) {
  assert(input_streams.size() > (unsigned int) rank);
  assert(input_streams[rank] != NULL);
  input_streams[rank]->set_length_time_slice(nBytes);
     
  set_status();
}

void Output_node::set_status() {
  if (status == END_NODE) return;
  status = STOPPED;
  if (!input_streams_order.empty()) status = WRITE_OUTPUT;
}

void Output_node::write_output() {
  assert(!input_streams_order.empty());
  
  int head = input_streams_order.begin()->second;
  
  assert(input_streams[head] != NULL);
  if (input_streams[head]->has_data()) {
    // Write data ...
    value_type &out_elem = data_writer_ctrl.buffer()->produce();
    int nBytes = input_streams[head]->write_bytes(out_elem);
    data_writer_ctrl.buffer()->produced(nBytes);
    
    // Check whether we arrived at the end of the slice
    if (input_streams[head]->end_of_slice()) {
      input_streams_order.erase(input_streams_order.begin());
    }
  } else {
    // No data available yet, sleep
    usleep(100000); // .1 second:
  }
}

bool Output_node::data_available() {
  return !input_streams_order.empty();
}

void Output_node::hook_added_data_reader(int reader) {
  // Create an output buffer:
  assert(data_readers_ctrl.get_buffer(reader) == NULL);
  Buffer *new_buffer = new Queue_buffer<value_type>();
  data_readers_ctrl.set_buffer(reader, new_buffer);
  
  // Create the data_stream:
  if (input_streams.size() <= (size_t)reader) {
    input_streams.resize(reader+1, NULL);
  }
  assert(input_streams[reader] == NULL);
  input_streams[reader] = new Input_stream(new Data_reader_buffer(new_buffer));
}

void Output_node::hook_added_data_writer(int writer) {
}


/*
 *  Input_stream
 */

Output_node::Input_stream::Input_stream(Data_reader *reader)
  : reader(reader), curr_bytes(0) 
{
}

int 
Output_node::Input_stream::write_bytes(value_type &elem) {
  if (curr_bytes == 0) {
    if (!slice_size.empty()) {
      curr_bytes = slice_size.front();
      slice_size.pop();
    }
  }
  if (curr_bytes == 0) {
    return 0;
  } else {
    int status = reader->get_bytes(min(131072, curr_bytes), elem.buffer());
    curr_bytes -= status;
    return status;
  }
}


bool 
Output_node::Input_stream::end_of_slice() {
  return (curr_bytes == 0);
}
bool 
Output_node::Input_stream::has_data() {
  return (curr_bytes > 0) || (!slice_size.empty());
}
    
void 
Output_node::Input_stream::set_length_time_slice(UINT64 nBytes) {
  slice_size.push(nBytes);
}
