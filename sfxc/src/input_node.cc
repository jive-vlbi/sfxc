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
                       TRANSPORT_TYPE transport_type, Time ref_date_):
    Node(rank, log_writer),
    input_node_ctrl(*this),
    data_reader_ctrl(*this),
    data_writers_ctrl(*this, MAX_TCP_CONNECTIONS),
    input_node_tasklet(NULL), status(WAITING),
    transport_type(transport_type), ref_date(ref_date_),
    station_number(station_number) {
  initialise();
}

Input_node::Input_node(int rank, int station_number,
                       TRANSPORT_TYPE transport_type, Time ref_date_) :
    Node(rank), input_node_ctrl(*this), data_reader_ctrl(*this),
    data_writers_ctrl(*this, MAX_TCP_CONNECTIONS),
    input_node_tasklet(NULL), status(WAITING),
    transport_type(transport_type), ref_date(ref_date_),
    station_number(station_number) {
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
  if(status==WAITING){
    SFXC_ASSERT(input_node_tasklet != NULL);
    input_node_tasklet->set_parameters(input_node_param, station_number);

    input_node_tasklet->start_tasklets();
  }
  status=WRITING;
}

Input_node::~Input_node() {
  if (input_node_tasklet != NULL)
    delete input_node_tasklet;
}

Time Input_node::get_time_stamp() {
  SFXC_ASSERT(input_node_tasklet != NULL);
  // the time in the tasklet is in micro seconds
  return input_node_tasklet->get_current_time();
}


void Input_node::start() {
  main_loop();
}

void Input_node::main_loop() {
  while ( status != END_NODE )
	{
    check_and_process_message();
  }
}

void Input_node::terminate() {
  DEBUG_MSG("WAITING FOR THE TASKLET TO FINISH");
  input_node_tasklet->stop_tasklets();
  input_node_tasklet->wait_termination();

  while (!data_writers_ctrl.ready())
    usleep(100000); // .1 second:

  int32_t rank = get_rank();
  MPI_Send(&rank, 1, MPI_INT32,
           RANK_MANAGER_NODE, MPI_TAG_DATASTREAM_EMPTY, MPI_COMM_WORLD);

  DEBUG_MSG("Input node terminate.");
  status = END_NODE;
}


void Input_node::hook_added_data_reader(size_t stream_nr) {
  SFXC_ASSERT(stream_nr == 0);

  input_node_tasklet =
    get_input_node_tasklet(data_reader_ctrl.get_data_reader(stream_nr),
                           transport_type, ref_date);
  SFXC_ASSERT(input_node_tasklet != NULL);
}

void Input_node::hook_added_data_writer(size_t writer) {}

void Input_node::add_time_interval(Time start_time, Time stop_time) {
  SFXC_ASSERT(input_node_tasklet != NULL);
  input_node_tasklet->add_time_interval(start_time, stop_time);
}

void Input_node::add_time_slice_to_stream(int channel, int stream, Time starttime_slice,
                                          Time stoptime_slice) {
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
