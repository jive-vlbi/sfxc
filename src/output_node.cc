/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */
#include "mpi_transfer.h"
#include "output_node.h"
#include "utils.h"

#include <iostream>

Output_node::Output_node(int rank, int size)
    : Node(rank),
    output_memory_pool(10),
    output_queue(new Output_queue()),
    output_node_ctrl(*this),
    data_readers_ctrl(*this),
    data_writer_ctrl(*this),
    status(STOPPED),
curr_slice(0), number_of_time_slices(-1), curr_stream(-1) {
  initialise();
}

Output_node::Output_node(int rank, Log_writer *writer, int size)
    : Node(rank, writer),
    output_memory_pool(10),
    output_queue(new Output_queue()),
    output_node_ctrl(*this),
    data_readers_ctrl(*this),
    data_writer_ctrl(*this),
    status(STOPPED),
curr_slice(0), number_of_time_slices(-1), curr_stream(-1) {
  initialise();
}

void Output_node::initialise() {
  add_controller(&data_readers_ctrl);
  add_controller(&output_node_ctrl);
  add_controller(&data_writer_ctrl);

  data_writer_ctrl.set_queue(output_queue);

	/// initialize and retreive the listening addresses/port
  std::vector<uint64_t> addrs;
	data_readers_ctrl.get_listening_ip( addrs );

	/// send this to the manager NODE
	MPI_Transfer::send_ip_address(addrs, RANK_MANAGER_NODE);


  int32_t msg=0;
  MPI_Send(&msg, 1, MPI_INT32,
           RANK_MANAGER_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD);
}

Output_node::~Output_node() {
  if (!get_assertion_raised()) {
    DEBUG_MSG("Output node finished");

    // empty the input buffers to the output
    SFXC_ASSERT(status == END_NODE);
    SFXC_ASSERT(input_streams_order.empty());


    // wait until the output buffer is empty (the memory pool is full)
    while (!output_memory_pool.full()) {
      usleep(10000);
    }
  }
}

void Output_node::terminate(){
	DEBUG_MSG("Output node terminate");
	status = END_NODE;
}

void Output_node::start() {
  while (status != END_NODE) {
    switch (status) {

      case STOPPED: {
        SFXC_ASSERT(curr_stream == -1);
        // blocking:
        check_and_process_message();

        if (curr_slice == number_of_time_slices) {
          status = END_NODE;
        } else if (!input_streams_order.empty()) {
          if (input_streams_order.begin()->first == curr_slice) {
            status = START_NEW_SLICE;
          }
        }
        break;
      }
      case START_NEW_SLICE: {
        SFXC_ASSERT(curr_stream == -1);

        SFXC_ASSERT(!input_streams_order.empty());
        SFXC_ASSERT(input_streams_order.begin()->first == curr_slice);
        curr_stream = input_streams_order.begin()->second;
        SFXC_ASSERT(curr_stream >= 0);
        input_streams_order.erase(input_streams_order.begin());
        input_streams[curr_stream]->goto_next_slice();

        status = WRITE_OUTPUT;
        break;
      }
      case WRITE_OUTPUT: {
        process_all_waiting_messages();

        if (!write_output()) {
          usleep(100);
        }

        // Check whether we arrived at the end of the slice
        if (input_streams[curr_stream]->end_of_slice()) {
          status = END_SLICE;
          break;
        }

        break;
      }
      case END_SLICE: {
        curr_stream = -1;
        curr_slice ++;
        if (curr_slice == number_of_time_slices) {
          status = END_NODE;
        } else if (input_streams_order.empty()) {
          status = STOPPED;
        } else if (input_streams_order.begin()->first != curr_slice) {
          status = STOPPED;
        } else {
          status = START_NEW_SLICE;
        }
        break;
      }
      case END_NODE: {
        // For completeness sake
        break;
      }
    }
  }

	DEBUG_MSG("WANT TO SHUT DOWN THE READER !");
	data_readers_ctrl.stop();
	DEBUG_MSG("WANT TO SHUT DOWN THE WRITER !");
	data_writer_ctrl.stop();
	DEBUG_MSG("WANT TO SHUT DOWN THE WRITER & READER !");

  // End the node;
  int32_t msg=0;
  MPI_Send(&msg, 1, MPI_INT32,
           RANK_MANAGER_NODE, MPI_TAG_OUTPUT_NODE_FINISHED, MPI_COMM_WORLD);
}

void
Output_node::
write_global_header(const Output_header_global &global_header) {
  SFXC_ASSERT(!output_memory_pool.empty());

  output_value_type element;
  element.actual_size = sizeof(Output_header_global);
  element.data = output_memory_pool.allocate();
  if (element.data->size() != 1000000)
    element.data->resize(1000000);
  memcpy(element.data->buffer(), (char *)&global_header,
         sizeof(Output_header_global));
  output_queue->push(element);
}

void
Output_node::
set_weight_of_input_stream(int stream, int64_t weight, size_t size) {
  SFXC_ASSERT(stream >= 0);

  SFXC_ASSERT(stream < (int)input_streams.size());
  // Check that the weight does not exist yet:
  SFXC_ASSERT(input_streams_order.find(weight) == input_streams_order.end());
  // Check that the ordering is right (not before the current element):
  if (!input_streams_order.empty()) {
    SFXC_ASSERT(weight >= curr_slice);
  }

  // Add the weight to the priority queue:
  input_streams_order.insert(Input_stream_priority_map_value(weight,stream));

  // Add the weight to the priority queue:
  input_streams[stream]->set_length_time_slice(size);

  SFXC_ASSERT(status != END_NODE);
}

bool Output_node::write_output() {
  if (output_memory_pool.empty())
    return false;

  SFXC_ASSERT(curr_stream >= 0);
  SFXC_ASSERT(input_streams[curr_stream] != NULL);
  SFXC_ASSERT(!output_memory_pool.empty());

  // Write data ...
  output_value_type element;
  element.data = output_memory_pool.allocate();
  input_streams[curr_stream]->write_bytes(element);
  if (element.actual_size <= 0)
    return false;

  output_queue->push(element);
  return true;
}

void Output_node::hook_added_data_reader(size_t reader) {
  // Create an output buffer:
  data_readers_ctrl.enable_buffering(reader);

  // Create the data_stream:
  if (input_streams.size() <= reader) {
    input_streams.resize(reader+1, NULL);
  }

  input_streams[reader] =
    new Input_stream(data_readers_ctrl.get_data_reader(reader));
}

void Output_node::hook_added_data_writer(size_t writer) {}

void
Output_node::set_number_of_time_slices(int n_time_slices) {
  number_of_time_slices = n_time_slices;
}


/*
 *  Input_stream
 */

Output_node::Input_stream::Input_stream(boost::shared_ptr<Data_reader> reader)
    : reader(reader) {
  reader->set_size_dataslice(0);
}

void
Output_node::Input_stream::write_bytes(output_value_type &elem) {
  SFXC_ASSERT(reader != boost::shared_ptr<Data_reader>());
  if (elem.data->size() != 1000000)
    elem.data->resize(1000000);
  size_t nBytes = std::min(elem.data->size(),
                           (size_t)reader->get_size_dataslice());
  elem.actual_size = reader->get_bytes(nBytes, elem.data->buffer());
}


bool
Output_node::Input_stream::end_of_slice() {
  return reader->end_of_dataslice();
}

void
Output_node::Input_stream::set_length_time_slice(int64_t nBytes) {
  slice_size.push(nBytes);
}

void
Output_node::Input_stream::goto_next_slice() {
  SFXC_ASSERT(!slice_size.empty());
  SFXC_ASSERT(slice_size.front() > 0);
  SFXC_ASSERT(reader->end_of_dataslice());
  reader->set_size_dataslice(slice_size.front());
  SFXC_ASSERT(reader->get_size_dataslice() > 0);
  slice_size.pop();
}
