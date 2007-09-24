/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: Node.cc 283 2007-07-12 12:13:17Z kruithof $
 *
 */

#include <Abstract_manager_node.h>
#include <utils.h>
#include <MPI_Transfer.h>
#include <assert.h>

Abstract_manager_node::
Abstract_manager_node(int rank, int numtasks, const Control_parameters &param)
  : Node(rank), control_parameters(param), numtasks(numtasks) {
}
Abstract_manager_node::
Abstract_manager_node(int rank, int numtasks,
                      Log_writer *writer, 
                      const Control_parameters &param)
  : Node(rank, writer), control_parameters(param), numtasks(numtasks) {
}
Abstract_manager_node::~Abstract_manager_node() {
}


// Start nodes:
void 
Abstract_manager_node::
start_input_node(int rank, const std::string &station) {
  input_node_rank[station] = rank;

  // starting an input reader
  MPI_Send(&rank, 1, MPI_INT32, 
           rank, MPI_TAG_SET_INPUT_NODE, MPI_COMM_WORLD);

  MPI_Status status;
  int msg;
  MPI_Recv(&msg, 1, MPI_INT32, 
           rank, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD,
           &status);
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
}
void
Abstract_manager_node::
start_correlator_node(int rank) {
  int correlator_node_nr = correlator_node_rank.size();
  correlator_node_rank.push_back(rank);

  // starting a correlator node
  MPI_Send(&correlator_node_nr, 1, MPI_INT32, rank,
           MPI_TAG_SET_CORRELATOR_NODE, MPI_COMM_WORLD);

  int msg;
  MPI_Status status;
  MPI_Recv(&msg, 1, MPI_INT32,
           rank, MPI_TAG_NODE_INITIALISED, 
           MPI_COMM_WORLD, &status);
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
set_single_data_reader(int rank, 
                       const std::string &filename) {
  int len = filename.size() +1; // for \0
  char c_filename[len];
  strcpy(c_filename, filename.c_str());
  c_filename[len-1] = '\0';
  
  MPI_Send(c_filename, len, MPI_CHAR, 
           rank, MPI_TAG_SET_DATA_READER_FILE, MPI_COMM_WORLD);
  
  wait_for_setting_up_channel(rank);
}

void
Abstract_manager_node::
set_multiple_data_reader(int rank, int stream_nr, 
                         const std::string &filename) {
  assert(false);
}

void
Abstract_manager_node::
set_single_data_writer(int rank, 
                       const std::string &filename) {
  assert(false);
}

void
Abstract_manager_node::
set_multiple_data_writer(int rank, int stream_nr, 
                         const std::string &filename) {
  assert(strncmp(filename.c_str(), "file://", 7) == 0);
  int len = 1 + filename.size() +1; // for \0
  char c_filename[len];
  snprintf(c_filename, len, "%c%s", (char)stream_nr, filename.c_str());
  assert(c_filename[len-1] == '\0');
  
  MPI_Send(c_filename, len, MPI_CHAR, 
           rank, MPI_TAG_ADD_DATA_WRITER_FILE, MPI_COMM_WORLD);
  
  wait_for_setting_up_channel(rank);
}

void 
Abstract_manager_node::
input_node_set(const std::string &station, Track_parameters &track_params) {
  MPI_Transfer transfer;
  transfer.send(track_params, input_rank(station));
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
input_node_goto_time(const std::string &station, int32_t time) {
  MPI_Send(&time, 1, MPI_INT32, 
           input_rank(station), MPI_TAG_INPUT_NODE_GOTO_TIME, MPI_COMM_WORLD);
}

void
Abstract_manager_node::
input_node_set_stop_time(const std::string &station, int32_t time) {
  MPI_Send(&time, 1, MPI_INT32, 
           input_rank(station), MPI_TAG_INPUT_NODE_STOP_TIME, MPI_COMM_WORLD);
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
  int64_t channel;
  if (rank >= 0) {
    MPI_Recv(&channel, 1, MPI_INT64, rank,
             MPI_TAG_INPUT_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &status);
  } else {
    MPI_Recv(&channel, 1, MPI_INT64, MPI_ANY_SOURCE,
             MPI_TAG_INPUT_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &status);
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

int 
Abstract_manager_node::
input_rank(const std::string &station) {
  assert(input_node_rank.find(station) != input_node_rank.end());
  return input_node_rank[station];
}

void
Abstract_manager_node::
correlator_node_set_all(Correlation_parameters &parameters) {
  assert(false);
}

void
Abstract_manager_node::
correlator_node_set_all(Delay_table_akima &delay_table) {
  assert(false);
}
