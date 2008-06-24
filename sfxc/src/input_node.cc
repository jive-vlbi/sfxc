/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#include "input_node.h"
#include "utils.h"
#include "types.h"
#include "data_reader_buffer.h"
#include "mpi_transfer.h"
#include "data_writer_file.h"

#include <iostream>
#include <time.h>
#include <math.h>

Input_node::Input_node(int rank,
                       int station_number,
                       Log_writer *log_writer,
                       TRANSPORT_TYPE transport_type) :
    Node(rank, log_writer),
    input_node_ctrl(*this),
    data_reader_ctrl(*this),
    data_writers_ctrl(*this, MAX_TCP_CONNECTIONS),
    input_node_tasklet(NULL), status(WAITING),
transport_type(transport_type) {
  initialise();
}

Input_node::Input_node(int rank, int station_number,
                       TRANSPORT_TYPE transport_type) :
    Node(rank), input_node_ctrl(*this), data_reader_ctrl(*this),
    data_writers_ctrl(*this, MAX_TCP_CONNECTIONS),
    input_node_tasklet(NULL), status(WAITING),
transport_type(transport_type) {
  initialise();
}
void Input_node::initialise() {
  get_log_writer()(1) << "Input_node()" << std::endl;
  add_controller(&input_node_ctrl);
  add_controller(&data_reader_ctrl);
  add_controller(&data_writers_ctrl);

	/// initialize and retreive the listening addresses/port
  std::vector<uint64_t> addrs;
	data_writers_ctrl.get_listening_ip( addrs );

	/// send this to the manager NODE
	MPI_Transfer::send_ip_address(addrs, RANK_MANAGER_NODE);

  int32_t msg;
  MPI_Send(&msg, 1, MPI_INT32, RANK_MANAGER_NODE, MPI_TAG_NODE_INITIALISED,
           MPI_COMM_WORLD);

}

void Input_node::set_input_node_parameters(const Input_node_parameters &input_node_param) {
  SFXC_ASSERT(input_node_tasklet != NULL);
  input_node_tasklet->set_parameters(input_node_param, get_rank()-3);
}

Input_node::~Input_node() {
  if (input_node_tasklet != NULL)
    delete input_node_tasklet;
}

int32_t Input_node::get_time_stamp() {
  SFXC_ASSERT(input_node_tasklet != NULL);
  // the time in the tasklet is in micro seconds
  return input_node_tasklet->get_current_time()/1000;
}

void Input_node::start() {
  while (status != END_NODE) {
    switch (status) {
      case WAITING: { // Wait until we can start sending new data
        // Wait for data_source to become ready
        if (check_and_process_message() == TERMINATE_NODE) {
          status = END_NODE;
          break;
        }
        if (input_node_tasklet != NULL)
          status = WRITING;
        break;
      }
      case WRITING: {
        if (process_all_waiting_messages() == TERMINATE_NODE) {
          status = END_NODE;
          break;
        }
        SFXC_ASSERT(input_node_tasklet != NULL);
        input_node_tasklet->do_task();
        if ( !input_node_tasklet->has_work() ) {
          usleep(1000);
          //status = WAITING;
        }
        break;
      }
      case END_NODE: {
         // For completeness sake
        break;
      }
    }
  }

  while (!data_writers_ctrl.ready())
    usleep(100000); // .1 second:

  int32_t rank = get_rank();
  MPI_Send(&rank, 1, MPI_INT32,
           RANK_MANAGER_NODE, MPI_TAG_DATASTREAM_EMPTY, MPI_COMM_WORLD);
}

void Input_node::hook_added_data_reader(size_t stream_nr) {
  SFXC_ASSERT(stream_nr == 0);

  input_node_tasklet =
    get_input_node_tasklet(data_reader_ctrl.get_data_reader(stream_nr),
                           transport_type);
  SFXC_ASSERT(input_node_tasklet != NULL);
}

void Input_node::hook_added_data_writer(size_t writer) {}

// Start time and stop time in seconds
void Input_node::add_time_interval(int32_t start_time, int32_t stop_time) {
  SFXC_ASSERT(input_node_tasklet != NULL);
  input_node_tasklet->add_time_interval(start_time, stop_time);
}

void Input_node::add_time_slice(int channel, int stream, int starttime_slice,
                                int stoptime_slice) {
  SFXC_ASSERT(data_writers_ctrl.get_data_writer(stream) !=
              Multiple_data_writers_controller::Data_writer_ptr());

  SFXC_ASSERT(input_node_tasklet != NULL);

  SFXC_ASSERT(stoptime_slice > starttime_slice);

  input_node_tasklet->add_data_writer(channel,
                                      data_writers_ctrl.get_data_writer(stream));
}

int Input_node::get_status() {
  return status;
}

void Input_node::set_delay_table(Delay_table_akima &delay_table) {
  SFXC_ASSERT(input_node_tasklet != NULL);
  input_node_tasklet->set_delay_table(delay_table);
}
