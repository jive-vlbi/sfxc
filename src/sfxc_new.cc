/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: test_Input_node.cc 285 2007-07-18 07:15:38Z kruithof $
 *
 *  Tests reading a file from disk and then writing it back using a Data_node
 */

#include <types.h>
#include <Input_node.h>
#include <Output_node.h>
#include <Log_node.h>
#include <Log_writer_cout.h>

#include <fstream>
#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#include "genFunctions.h"
#include "InData.h"
#include "delayTable.h"
#include "MPI_Transfer.h"


#include <Node.h>
#include <Data_reader2buffer.h>
#include <TCP_Connection.h>
#include <Buffer2data_writer.h>
#include <Data_writer.h>
#include <Data_writer_file.h>
#include <Data_reader_file.h>
#include <Data_reader_tcp.h>
#include <Channel_extractor_mark4.h>
#include <utils.h>

#include <Abstract_manager_node.h>

const int input_node = 2;
const int output_node = 3;
const char output_file_to_disk[] = "output_file_to_disk.dat";

int64_t start_time = -1;
int64_t stop_time  = -1;

class Test_manager_node : Abstract_manager_node {
public:
  Test_manager_node(int rank, 
                    Log_writer *log_writer,
                    const Control_parameters &control_parameters,
                    char *tmp_dir);

  void start();


  void hook_added_data_reader(size_t reader) {};
  void hook_added_data_writer(size_t writer) {};

private:
  char *tmp_dir;
};

Test_manager_node::
Test_manager_node(int rank, 
                  Log_writer *log_writer,
                  const Control_parameters &control_parameters,
                  char *tmp_dir)
  : Abstract_manager_node(rank, 
                          log_writer,
                          control_parameters),
    tmp_dir(tmp_dir)
{
}

void Test_manager_node::start() {
  get_log_writer()(0) << "Starting nodes" << std::endl;
  start_log_node(RANK_LOG_NODE);
  start_input_node(input_node, 0);
  start_output_node(output_node);

  get_log_writer()(0) << "Initialising the Input_node" << std::endl;
  // setting the first data-source of the first station
  const std::string &station_name = control_parameters.station(0);
  std::string filename = control_parameters.data_sources(station_name)[0];
  set_single_data_reader(input_node, filename);

  // Send the track parameters
  std::vector<std::string> scans;
  control_parameters.get_vex().get_scans(std::back_inserter(scans));
  const std::string &mode = 
    control_parameters.get_vex().get_mode(scans[0]);
  const std::string &track = 
    control_parameters.get_vex().get_track(mode, station_name);
  Track_parameters track_param =
    control_parameters.get_track_parameters(track);
  set_track_parameters(input_node, track_param);

  // Set the output nodes
  {
    int channel_nr =0;
    for (Track_parameters::Channel_iterator 
           chan_it = track_param.channels.begin();
         chan_it != track_param.channels.end(); chan_it++, channel_nr++) {
      char filename[80];
      snprintf(filename, 80, "file:///tmp/test_Input_node_output%d.out",
               channel_nr);
      set_multiple_data_writer(input_node, channel_nr, filename);
    }
    for (Track_parameters::Channel_iterator 
           chan_it = track_param.channels.begin();
         chan_it != track_param.channels.end(); chan_it++, channel_nr++) {
      char filename[80];
      snprintf(filename, 80, "file:///tmp/test_Input_node_output%d.out",
               channel_nr);
      set_multiple_data_writer(input_node, channel_nr, filename);
    }
  }

  // Setting the start time
  int64_t current_time = input_node_get_current_time(input_node);
  int32_t start_time = control_parameters.get_start_time().to_miliseconds();
  int32_t stop_time = start_time + 2000; // add two seconds
  assert(current_time < start_time);
  // goto the start time 
  input_node_goto_time(input_node, start_time);
  // set the stop time
  input_node_set_stop_time(input_node, stop_time);

  get_log_writer()(0) << "Writing a time slice" << std::endl;
  if ( /* one_slice */ true ) {
    int channel_nr =0;
    for (Track_parameters::Channel_iterator 
           chan_it = track_param.channels.begin();
         chan_it != track_param.channels.end(); chan_it++, channel_nr++) {
      input_node_set_time_slice(input_node, channel_nr, /*stream*/channel_nr,
                                start_time, stop_time);
    }
  } else {
    int channel_nr =0;
    for (Track_parameters::Channel_iterator 
           chan_it = track_param.channels.begin();
         chan_it != track_param.channels.end(); chan_it++, channel_nr++) {
      input_node_set_time_slice(input_node, channel_nr, /*stream*/channel_nr,
                                start_time, start_time+1000);
    }
    for (Track_parameters::Channel_iterator 
           chan_it = track_param.channels.begin();
         chan_it != track_param.channels.end(); chan_it++, channel_nr++) {
      input_node_set_time_slice(input_node, 
                                channel_nr-track_param.channels.size(),
                                /*stream*/channel_nr,
                                start_time+1000, stop_time);
    }
  }

  // Waiting for the input node to finish
  int status=get_status(input_node);
  while (status != Input_node::WRITING) {
    usleep(100000); // .1 second
    status = get_status(input_node);
  }
  while (status == Input_node::WRITING) {
    usleep(100000); // .1 second
    status = get_status(input_node);
  }
  
  get_log_writer()(0) << "Terminating nodes" << std::endl;
  end_node(input_node);
  end_node(output_node);
  end_node(RANK_LOG_NODE);
}

int main(int argc, char *argv[]) {
  assert(input_node+output_node+RANK_MANAGER_NODE+RANK_LOG_NODE == 6);

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
  assert(numtasks == 4);
  
  assert(argc == 4);
  char *ctrl_file = argv[1];
  char *vex_file = argv[2];
  char *output_directory = argv[3];

  Log_writer_mpi log_writer(rank, 1);
  //Log_writer_cout log_writer(1);
  if (rank == RANK_MANAGER_NODE) {
    Control_parameters control_parameters;
    control_parameters.initialise(ctrl_file, vex_file, log_writer);
    Test_manager_node node(rank, &log_writer,
                           control_parameters,
                           output_directory);
    node.start();
  } else {
    start_node();
  }

  //close the mpi stuff
  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Finalize();

  return 0;
}
