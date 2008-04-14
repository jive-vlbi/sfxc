/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id: Output_node_without_buffering.cc 222 2007-05-14 07:22:32Z kruithof $
 *
 */

#include <Output_node_without_buffering.h>

#include <types.h>
#include <Data_writer_file.h>
#include <Data_reader_buffer.h>
#include <Queue_buffer.h>

#include <iostream>
#include <assert.h>

Output_node_without_buffering::Output_node_without_buffering(int rank, int size)
    : Node(rank),
    output_buffer(1000),
    Output_node_without_buffering_ctrl(*this),
    data_readers_ctrl(*this),
    data_writer_ctrl(*this),
    status(STOPPED),
    curr_slice(0), number_of_time_slices(-1) {
  initialise();
}

Output_node_without_buffering::Output_node_without_buffering(int rank, Log_writer *writer, int size)
    : Node(rank, writer),
    output_buffer(1000),
    Output_node_without_buffering_ctrl(*this),
    data_readers_ctrl(*this),
    data_writer_ctrl(*this),
    curr_slice(0), number_of_time_slices(-1) {
  initialise();
}

void Output_node_without_buffering::initialise() {
  add_controller(&data_readers_ctrl);
  add_controller(&Output_node_without_buffering_ctrl);
  add_controller(&data_writer_ctrl);

  int32_t msg;
  MPI_Send(&msg, 1, MPI_INT32,
           RANK_MANAGER_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD);

  data_writer_ctrl.set_buffer(&output_buffer);
}

Output_node_without_buffering::~Output_node_without_buffering() {
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

void Output_node_without_buffering::start() {
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

        // check incoming connections
        for (size_t reader = 0;
             reader < data_readers_ctrl.number_of_data_readers();
             reader++) {
          if (data_readers_ctrl.get_data_reader2buffer(reader)->get_state() ==
              Multiple_data_readers_controller::Reader2buffer::SUSPENDED) {
            get_log_writer() << "Data_reader " << reader << " suspended" << std::endl;
            data_readers_ctrl.get_data_reader2buffer(reader)->set_state(
              Multiple_data_readers_controller::Reader2buffer::STOPPED);
          }
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
    if (curr_slice == number_of_time_slices) {
      status = END_NODE;
    }
  }
}

void Output_node_without_buffering::create_buffer(int num) {
}

void Output_node_without_buffering::set_weight_of_input_stream(int num, uint64_t weight) {
  assert(num >= 0);

  // Add the weight to the priority queue:
  // Check that the weight does not exist yet:
  assert(input_streams_order.find(weight) == input_streams_order.end());
  input_streams_order.insert(Input_stream_priority_map_value(weight,num));
}

void Output_node_without_buffering::time_slice_finished(int rank, uint64_t nBytes) {}

void Output_node_without_buffering::set_status() {}

void Output_node_without_buffering::write_output() {
  assert(data_available());

  int reader_nr = input_streams_order.begin()->second;
  int n_bytes =
    std::min(bytes_in_timeslice_per_input_stream[reader_nr],
             131072);
  char buffer[n_bytes];
  int bytes_read =
    data_readers_ctrl.get_data_reader(reader_nr)->get_bytes(n_bytes, buffer);
  bytes_in_timeslice_per_input_stream[reader_nr] -= bytes_read;
  data_writer_ctrl.get_data_writer(0)->put_bytes(bytes_read, buffer);
}

bool Output_node_without_buffering::data_available() {
  if (input_streams_order.empty()) return false;
  return (bytes_in_timeslice_per_input_stream[input_streams_order.begin()->second] > 0);
}

void Output_node_without_buffering::hook_added_data_reader(size_t reader) {}

void Output_node_without_buffering::hook_added_data_writer(size_t writer) {}

void
Output_node_without_buffering::set_number_of_time_slices(int n_time_slices) {
  number_of_time_slices = n_time_slices;
}


