/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 *  Tests reading a file from disk and then writing it back using a Data_node
 */

#include <fstream>
#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#include "types.h"
#include "input_node.h"
#include "log_node.h"
#include "log_writer_cout.h"

#include "mpi_transfer.h"

#include "node.h"
#include "data_reader2buffer.h"
#include "tcp_connection.h"
#include "buffer2data_writer.h"
#include "data_writer.h"
#include "data_writer_file.h"
#include "data_reader_file.h"
#include "data_reader_tcp.h"
#include "channel_extractor_mark4.h"
#include "utils.h"

#include "abstract_manager_node.h"

const int rank_input_node = 2;
const char output_file_to_disk[] = "output_file_to_disk.dat";

int64_t start_time = -1;
int64_t stop_time  = -1;

class Test_manager_node : Abstract_manager_node {
public:
  Test_manager_node(int rank, int numtasks,
                    Log_writer *log_writer,
                    const Control_parameters &control_parameters,
                    char *tmp_dir,
                    char *file_basename,
                    int nr_time_slices);

  void start();


  void hook_added_data_reader(size_t reader) {};
  void hook_added_data_writer(size_t writer) {};

private:
  char *tmp_dir, *file_basename;
  int nr_time_slices;
};

Test_manager_node::
Test_manager_node(int rank, int numtasks,
                  Log_writer *log_writer,
                  const Control_parameters &control_parameters,
                  char *tmp_dir,
                  char *file_basename,
                  int nr_time_slices)
    : Abstract_manager_node(rank, numtasks,
                            log_writer,
                            control_parameters),
    tmp_dir(tmp_dir), file_basename(file_basename),
    nr_time_slices(nr_time_slices) {}

void Test_manager_node::start() {
  get_log_writer()(0) << "Starting nodes" << std::endl;
  start_log_node(RANK_LOG_NODE);

  get_log_writer()(0) << "Initialising the Input_node" << std::endl;


  // setting the first data-source of the first station
  const std::string &station_name = control_parameters.station(0);
  start_input_node(rank_input_node, station_name);
  std::string filename = control_parameters.data_sources(station_name)[0];
  set_data_reader(rank_input_node, /*stream_nr*/0, filename);

  // Sending the delay table
  Delay_table_akima delay_table;
  const std::string &delay_file =
    control_parameters.get_delay_table_name(station_name);
  delay_table.open(delay_file.c_str());
  if (!delay_table.initialised()) {
    DEBUG_MSG("Delay table could not be read");
    control_parameters.generate_delay_table(station_name, delay_file);
    delay_table.open(delay_file.c_str());
    assert(delay_table.initialised());
  }

  // Send the track parameters
  std::vector<std::string> scans;
  control_parameters.get_vex().get_scans(std::back_inserter(scans));
  const std::string &mode =
    control_parameters.get_vex().get_mode(scans[0]);
  Input_node_parameters input_node_param =
    control_parameters.get_input_node_parameters(mode, station_name);
  input_node_set(station_name, input_node_param);

  // Set the output nodes
  {
    int channel_nr =0;
    for (int time_slice=0; time_slice < nr_time_slices; time_slice++) {
      for (Input_node_parameters::Channel_iterator
           chan_it = input_node_param.channels.begin();
           chan_it != input_node_param.channels.end(); chan_it++, channel_nr++) {
        char filename[80];
        snprintf(filename, 80, "file://%s/%s_%02d_%02d.out",
                 tmp_dir, file_basename, channel_nr, time_slice);
        set_data_writer(rank_input_node, channel_nr, filename);
      }
    }
  }

  get_log_writer()(0) << "Opened the output streams" << std::endl;

  // Setting the start time
  int64_t current_time = input_node_get_current_time(station_name);
  int32_t start_time = control_parameters.get_start_time().to_miliseconds();
  int32_t stop_time = start_time + 2000; // add two seconds
  if (current_time > start_time) {
    get_log_writer()(0) << "Error: current time after start time" << std::endl;
    return;
  }
  // goto the start time
  input_node_goto_time(station_name, start_time);
  // set the stop time
  input_node_set_stop_time(station_name, stop_time);

  get_log_writer()(0) << "Writing a time slice" << std::endl;
  {
    int delta_time = 500; // Length of a time slice in milliseconds
    assert(start_time+delta_time*(nr_time_slices-1) < stop_time);
    int channel_nr =0;
    for (int time_slice=0; time_slice < nr_time_slices-1; time_slice++) {
      for (Input_node_parameters::Channel_iterator
           chan_it = input_node_param.channels.begin();
           chan_it != input_node_param.channels.end(); chan_it++, channel_nr++) {
        input_node_set_time_slice(station_name,
                                  channel_nr%input_node_param.channels.size(),
                                  /*stream*/channel_nr,
                                  start_time+time_slice*delta_time,
                                  start_time+(time_slice+1)*delta_time);
      }
    }
    for (Input_node_parameters::Channel_iterator
         chan_it = input_node_param.channels.begin();
         chan_it != input_node_param.channels.end(); chan_it++, channel_nr++) {
      input_node_set_time_slice(station_name,
                                channel_nr%input_node_param.channels.size(),
                                /*stream*/channel_nr,
                                start_time+(nr_time_slices-1)*delta_time,
                                stop_time);
    }
  }

  // Waiting for the input node to finish
  int status=get_status(rank_input_node);
  while (status != Input_node::WRITING) {
    usleep(100000); // .1 second
    status = get_status(rank_input_node);
  }

  while (status == Input_node::WRITING) {
    usleep(100000); // .1 second
    status = get_status(rank_input_node);
  }

  get_log_writer()(0) << "Terminating nodes" << std::endl;
  end_node(rank_input_node);
  end_node(RANK_LOG_NODE);
}

int main(int argc, char *argv[]) {
  assert(rank_input_node+RANK_MANAGER_NODE+RANK_LOG_NODE == 3);

  //initialisation
  int stat = MPI_Init(&argc,&argv);
  if (stat != MPI_SUCCESS) {
    std::cout << "Error starting MPI program. Terminating.\n";
    MPI_Abort(MPI_COMM_WORLD, stat);
  }

  // MPI
  int numtasks, rank;
  // get the number of tasks set at commandline (= number of processors)
  MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
  // get the ID (rank) of the task, fist rank=0, second rank=1 etc.
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  assert(numtasks == 3);

  assert(argc == 4);
  char *ctrl_file = argv[1];
  char *vex_file = argv[2];
  char *output_directory = argv[3];

  Log_writer_mpi log_writer(rank, 1);
  //Log_writer_cout log_writer(1);
  if (rank == RANK_MANAGER_NODE) {
    Control_parameters control_parameters;
    control_parameters.initialise(ctrl_file, vex_file, log_writer);
    char *base_filename = "test_Input_node";
    Test_manager_node node(rank, numtasks, &log_writer,
                           control_parameters,
                           output_directory, base_filename,
                           /* nr of time slices */ 1);
    node.start();
  } else {
    start_node();
  }

  MPI_Barrier( MPI_COMM_WORLD );

  //Log_writer_cout log_writer(1);
  if (rank == RANK_MANAGER_NODE) {
    Control_parameters control_parameters;
    control_parameters.initialise(ctrl_file, vex_file, log_writer);
    char *base_filename = "test_Input_node_two_slices";
    Test_manager_node node(rank, numtasks, &log_writer,
                           control_parameters,
                           output_directory, base_filename,
                           /* nr of time slices */ 2);
    node.start();
  } else {
    start_node();
  }

  //close the mpi stuff
  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Finalize();

  return 0;
}
