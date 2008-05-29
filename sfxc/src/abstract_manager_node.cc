/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id: Node.cc 283 2007-07-12 12:13:17Z kruithof $
 *
 */

#include <assert.h>
#include <iostream>

#include "abstract_manager_node.h"
#include "mpi_transfer.h"
#include "utils.h"

Abstract_manager_node::
Abstract_manager_node(int rank, int numtasks, const Control_parameters &param)
    : Node(rank), control_parameters(param), numtasks(numtasks) {}
Abstract_manager_node::
Abstract_manager_node(int rank, int numtasks,
                      Log_writer *writer,
                      const Control_parameters &param)
    : Node(rank, writer), control_parameters(param), numtasks(numtasks) {}
Abstract_manager_node::~Abstract_manager_node() {}


// Start nodes:
void
Abstract_manager_node::
start_input_node(int rank, const std::string &station) {
  input_node_map[station] = input_node_rank.size();
  input_node_rank.push_back(rank);

  if (control_parameters.transport_type(station) == "Mark5A") {
    // starting an input reader
    MPI_Send(&rank, 1, MPI_INT32,
             rank, MPI_TAG_SET_INPUT_NODE_MARK5A, MPI_COMM_WORLD);
  } else {
    assert(control_parameters.transport_type(station) == "Mark5B");
    // starting an input reader
    MPI_Send(&rank, 1, MPI_INT32,
             rank, MPI_TAG_SET_INPUT_NODE_MARK5B, MPI_COMM_WORLD);
  }

  MPI_Status status;
  int msg;
  MPI_Recv(&msg, 1, MPI_INT32,
           rank, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD,
           &status);

  set_message_level(rank, get_log_writer().get_maxlevel());
}
void
Abstract_manager_node::
start_output_node(int rank) {
  assert(rank == RANK_OUTPUT_NODE);
  // starting an input reader
  int32_t msg=0;
  MPI_Send(&msg, 1, MPI_INT32,
           rank, MPI_TAG_SET_OUTPUT_NODE, MPI_COMM_WORLD);

  MPI_Status status;
  MPI_Recv(&msg, 1, MPI_INT32,
           rank, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD,
           &status);

  set_message_level(rank, get_log_writer().get_maxlevel());
}
void
Abstract_manager_node::
start_correlator_node(int rank) {
  size_t correlator_node_nr = correlator_node_rank.size();
#ifdef SFXC_DETERMINISTIC
  correlator_node_ready.resize(correlator_node_nr+1);
  assert(correlator_node_nr < correlator_node_ready.size());
  set_correlator_node_ready(correlator_node_nr, false);
#endif
  correlator_node_rank.push_back(rank);

  // starting a correlator node
  MPI_Send(&correlator_node_nr, 1, MPI_INT32, rank,
           MPI_TAG_SET_CORRELATOR_NODE, MPI_COMM_WORLD);

  int msg;
  MPI_Status status;
  MPI_Recv(&msg, 1, MPI_INT32,
           rank, MPI_TAG_NODE_INITIALISED,
           MPI_COMM_WORLD, &status);

  set_message_level(rank, get_log_writer().get_maxlevel());
}

void
Abstract_manager_node::
start_log_node(int rank) {
  assert(rank == RANK_LOG_NODE);
  int msg=0;
  // Log node:
  MPI_Send(&msg, 1, MPI_INT32,
           RANK_LOG_NODE, MPI_TAG_SET_LOG_NODE, MPI_COMM_WORLD);
  MPI_Send(&msg, 1, MPI_INT32,
           RANK_LOG_NODE, MPI_TAG_LOG_NODE_SET_OUTPUT_COUT, MPI_COMM_WORLD);

  MPI_Status status;
  MPI_Recv(&msg, 1, MPI_INT32,
           RANK_LOG_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD, &status);
  set_message_level(rank, get_log_writer().get_maxlevel());
}

void
Abstract_manager_node::
start_log_node(int rank, char *filename) {
  assert(rank == RANK_LOG_NODE);
  int msg=0;
  // Log node:
  MPI_Send(&msg, 1, MPI_INT32,
           RANK_LOG_NODE, MPI_TAG_SET_LOG_NODE, MPI_COMM_WORLD);
  MPI_Send(filename, strlen(filename)+1, MPI_CHAR,
           RANK_LOG_NODE, MPI_TAG_LOG_NODE_SET_OUTPUT_FILE, MPI_COMM_WORLD);

  MPI_Status status;
  MPI_Recv(&msg, 1, MPI_INT32,
           RANK_LOG_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD, &status);

  set_message_level(rank, get_log_writer().get_maxlevel());
}

void
Abstract_manager_node::set_message_level(int rank, int32_t messagelevel) {
  MPI_Send(&messagelevel, 1, MPI_INT32,
           rank, MPI_TAG_SET_MESSAGELEVEL, MPI_COMM_WORLD);
}

void
Abstract_manager_node::
end_node(int rank) {
  int32_t type = 0;
  MPI_Send(&type, 1, MPI_INT32,
           rank, MPI_TAG_END_NODE, MPI_COMM_WORLD);
}

int
Abstract_manager_node::
get_status(int rank) {
  int32_t result = 0;
  MPI_Send(&result, 1, MPI_INT32,
           rank, MPI_TAG_GET_STATUS, MPI_COMM_WORLD);

  MPI_Status status;
  MPI_Recv(&result, 1, MPI_INT32,
           rank, MPI_TAG_GET_STATUS, MPI_COMM_WORLD, &status);
  return result;
}

/// Setting of the data readers/writers
void
Abstract_manager_node::
set_data_reader(int rank, int32_t stream_nr,
                const std::string &filename) {
  //DEBUG_MSG(filename << " => " << rank<< "[" << stream_nr << "]");
  int len = sizeof(int32_t)+filename.size() +1; // for \0
  char msg[len];
  memcpy(msg,&stream_nr,sizeof(int32_t));
  memcpy(msg+sizeof(int32_t), filename.c_str(), filename.size()+1);

  MPI_Send(msg, len, MPI_CHAR,
           rank, MPI_TAG_ADD_DATA_READER, MPI_COMM_WORLD);

  wait_for_setting_up_channel(rank);
}

void Abstract_manager_node::
set_data_writer_void(int writer_rank, int writer_stream_nr) {
  //DEBUG_MSG(writer_rank<< "[" << writer_stream_nr << "] => void");
  MPI_Send(&writer_stream_nr, 1, MPI_INT32,
           writer_rank, MPI_TAG_ADD_DATA_WRITER_VOID2, MPI_COMM_WORLD);
  wait_for_setting_up_channel(writer_rank);
}

void
Abstract_manager_node::
set_data_writer(int rank, int stream_nr,
                const std::string &filename) {
  //DEBUG_MSG(rank << "[" << stream_nr << "] => " << filename);
  assert(strncmp(filename.c_str(), "file://", 7) == 0);
  int len = sizeof(int32_t) + filename.size() +1; // for \0
  char msg[len];
  memcpy(msg,&stream_nr,sizeof(int32_t));
  memcpy(msg+sizeof(int32_t), filename.c_str(), filename.size()+1);
  assert(msg[len-1] == '\0');

  MPI_Send(msg, len, MPI_CHAR,
           rank, MPI_TAG_ADD_DATA_WRITER_FILE2, MPI_COMM_WORLD);

  wait_for_setting_up_channel(rank);
}

void
Abstract_manager_node::set_TCP(int writer_rank, int writer_stream_nr,
                               int reader_rank, int reader_stream) {
  // DEBUG_MSG(writer_rank << "[" << writer_stream_nr << "] => "
  //          << reader_rank << "[" << reader_stream << "]");
  int32_t msg[3] = {writer_stream_nr, reader_rank, reader_stream};
  MPI_Send(msg, 3, MPI_INT32,
           writer_rank, MPI_TAG_ADD_TCP, MPI_COMM_WORLD);

  wait_for_setting_up_channel(writer_rank);
}

void
Abstract_manager_node::
input_node_set(const std::string &station, Input_node_parameters &input_node_params) {
  MPI_Transfer::send(input_node_params, input_rank(station));
}

int32_t
Abstract_manager_node::
input_node_get_current_time(const std::string &station) {
  int rank = input_rank(station);
  int32_t result;
  MPI_Send(&result, 1, MPI_INT32,
           rank, MPI_TAG_INPUT_NODE_GET_CURRENT_TIMESTAMP, MPI_COMM_WORLD);
  MPI_Status status;
  MPI_Recv(&result, 1, MPI_INT32, rank,
           MPI_TAG_INPUT_NODE_GET_CURRENT_TIMESTAMP, MPI_COMM_WORLD, &status);
  return result;
}

void
Abstract_manager_node::
input_node_set_time(const std::string &station, 
                     int32_t start_time, int32_t stop_time) {
  assert(start_time < stop_time);
  int32_t time[2];
  time[0] = start_time;
  time[1] = stop_time;
  MPI_Send(&time[0], 2, MPI_INT32,
           input_rank(station), MPI_TAG_INPUT_NODE_SET_TIME, MPI_COMM_WORLD);
}

void
Abstract_manager_node::
input_node_set_time_slice(const std::string &station,
                          int32_t channel, int32_t stream_nr,
                          int32_t start_time, int32_t stop_time) {
  int32_t message[] = {channel, stream_nr, start_time, stop_time};
  MPI_Send(&message, 4, MPI_INT32,
           input_rank(station),
           MPI_TAG_INPUT_NODE_ADD_TIME_SLICE, MPI_COMM_WORLD);
}

void
Abstract_manager_node::
wait_for_setting_up_channel(int rank) {
  MPI_Status status;
  int32_t channel;
  if (rank >= 0) {
    MPI_Recv(&channel, 1, MPI_INT32, rank,
             MPI_TAG_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &status);
  } else {
    MPI_Recv(&channel, 1, MPI_INT32, MPI_ANY_SOURCE,
             MPI_TAG_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &status);
  }
}

const Control_parameters &
Abstract_manager_node::
get_control_parameters() const {
  return control_parameters;
}

int
Abstract_manager_node::
get_number_of_processes() const {
  return numtasks;
}

size_t
Abstract_manager_node::
number_correlator_nodes() const {
  return correlator_node_rank.size();
}

size_t
Abstract_manager_node::
input_node(const std::string &station) const {
  std::map<std::string, int>::const_iterator it = input_node_map.find(station);
  assert(it != input_node_map.end());
  return it->second;
}
size_t
Abstract_manager_node::
input_rank(size_t input_node_nr) const {
  assert(input_node_nr < input_node_rank.size());
  return input_node_rank[input_node_nr];
}
size_t
Abstract_manager_node::
input_rank(const std::string &station_name) const {
  return input_rank(input_node(station_name));
}

void
Abstract_manager_node::
correlator_node_set(Correlation_parameters &parameters,
                    int corr_node_nr) {
  MPI_Transfer::send(parameters,correlator_node_rank[corr_node_nr]);
}

void
Abstract_manager_node::
correlator_node_set_all(Delay_table_akima &delay_table,
                        const std::string &station_name) {
  for (size_t i=0; i<correlator_node_rank.size(); i++) {
    send(delay_table,
         input_node(station_name),
         correlator_node_rank[i]);
  }

  // Cross polarize?
  if (control_parameters.cross_polarize()) {
    int nStations = control_parameters.number_stations();
    for (size_t i=0; i<correlator_node_rank.size(); i++) {
      send(delay_table,
           input_node(station_name)+nStations,
           correlator_node_rank[i]);
    }
  }
}

void
Abstract_manager_node::
set_correlator_node_ready(size_t correlator_nr, bool ready) {
#ifdef SFXC_DETERMINISTIC
  assert(correlator_nr < correlator_node_ready.size());
  correlator_node_ready[correlator_nr] = ready;
#else
  if (ready) {
    ready_correlator_nodes.push(correlator_nr);
  }
#endif
}


void
Abstract_manager_node::
send(Delay_table_akima &delay_table, int station, int to_rank) {
  MPI_Transfer::send(delay_table, station, to_rank);

}
const std::map<std::string, int> &
Abstract_manager_node::get_input_node_map() const {
  return input_node_map;
}

size_t
Abstract_manager_node::get_channel(const std::string &channel) {
  for (size_t channel_nr = 0;
       channel_nr < control_parameters.number_frequency_channels();
       channel_nr++) {
    if (channel == control_parameters.frequency_channel(channel_nr)) {
      return channel_nr;
    }
  }
  // error message
  assert(false);
  return control_parameters.number_frequency_channels();
}

void
Abstract_manager_node::
output_node_set_global_header(char* header_msg, int size) {
  MPI_Send(header_msg, size, MPI_CHAR,
           RANK_OUTPUT_NODE,
           MPI_TAG_OUTPUT_NODE_GLOBAL_HEADER,
           MPI_COMM_WORLD);
}
