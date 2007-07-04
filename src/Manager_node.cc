/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <Manager_node.h>
#include <iostream>
#include <assert.h>

#include "sfxc_mpi.h"
#include "utils.h"
#include "MPI_Transfer.h"

// NGHK: Global variables
#include <runPrms.h>
#include <genPrms.h>
#include <staPrms.h>
#include <constPrms.h>
extern RunP RunPrms;
extern GenP GenPrms;
extern StaP StaPrms[NstationsMax];

Manager_node::Manager_node(int numtasks, int rank, char * control_file) 
  : Node(rank), numtasks(numtasks), slicenr(0), 
    manager_controller(*this)
{
  get_log_writer() << "Manager_node()" << std::endl;
  MPI_Status status;
  
  assert(rank == 0);

  add_controller(&manager_controller);

  { // Initialise the log node, otherwise no error messages can be sent.
    assert (RANK_LOG_NODE == 1);
    
    int msg=0;
    // Log node:
    MPI_Send(&msg, 1, MPI_INT32, 
             RANK_LOG_NODE, MPI_TAG_SET_LOG_NODE, MPI_COMM_WORLD);
    MPI_Send(&msg, 1, MPI_INT32, 
             RANK_LOG_NODE, MPI_TAG_LOG_NODE_SET_OUTPUT_COUT, MPI_COMM_WORLD);
    MPI_Recv(&msg, 1, MPI_INT32, 
             RANK_LOG_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD, &status);
  }  

  
  int err;
  err = read_control_file(control_file);
  if (err != 0) return;

  START_INPUT_NODES = 3;
#ifdef READ_DATA_FOR_CORRELATE_NODES_FROM_FILE
  // Reading from file does not need input nodes:
  N_INPUT_NODES = 0;
#else
  N_INPUT_NODES = GenPrms.get_nstations();
  assert(N_INPUT_NODES > 0);
#endif
  
  START_CORRELATE_NODES = START_INPUT_NODES + N_INPUT_NODES;
  N_CORRELATE_NODES = numtasks - START_CORRELATE_NODES;

  assert(N_CORRELATE_NODES > 1);

  { // Initialise the output node
    assert (RANK_OUTPUT_NODE == 2);
    
    int msg=0;
    const char *filename = GenPrms.get_corfile();
    MPI_Send(&msg, 1, MPI_INT32, 
             RANK_OUTPUT_NODE, MPI_TAG_SET_OUTPUT_NODE, MPI_COMM_WORLD);
    MPI_Send((void *)filename, strlen(filename)+1, MPI_CHAR, 
             RANK_OUTPUT_NODE, MPI_TAG_SET_DATA_WRITER_FILE, MPI_COMM_WORLD);

    int64_t channel;
    MPI_Recv(&channel, 1, MPI_INT64, MPI_ANY_SOURCE,
             MPI_TAG_INPUT_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &status);
    assert((int)status.MPI_SOURCE == RANK_OUTPUT_NODE);               

    MPI_Recv(&msg, 1, MPI_INT32, 
             RANK_OUTPUT_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD, &status);
  }  

  // set all nodes to INITIALISING:
  state_correlate_nodes.resize(numtasks, INITIALISING);

  // Nstations+3 and later are correlate nodes
  {
    int n_nodes_initialised=0;
    bool connection[numtasks];
    for (int i=0; i<numtasks; i++) connection[i] = false;

    for (int i=0; i<N_CORRELATE_NODES; i++) {
      // starting a correlator node
      MPI_Send(&i, 1, MPI_INT32, i+START_CORRELATE_NODES, 
               MPI_TAG_SET_CORRELATOR_NODE, MPI_COMM_WORLD);
      int msg;
      MPI_Recv(&msg, 1, MPI_INT32, 
               i+START_CORRELATE_NODES, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD, &status);
  
      err = send_control_parameters_to_controller_node(i+START_CORRELATE_NODES);
      if (err != 0) return;
  
      
      // Set the output node:
#ifdef USE_FILES_FOR_OUTPUT                
      char output_filename[80];
      sprintf(output_filename, "%s.%d", GenPrms.get_corfile(), i);
      MPI_Send(output_filename, strlen(output_filename)+1, MPI_CHAR,
               i+START_CORRELATE_NODES, MPI_TAG_SET_DATA_WRITER_FILE, MPI_COMM_WORLD);
#else
      int32_t ranks[2] = {i, RANK_OUTPUT_NODE};
      MPI_Send(ranks, 2, MPI_INT32, 
               i+START_CORRELATE_NODES, 
               MPI_TAG_SET_OUTPUT_CONNECTION_MULTIPLE_INPUT_TCP, MPI_COMM_WORLD);
#endif

      int result;
      MPI_Status stat;
      MPI_Iprobe(MPI_ANY_SOURCE, MPI_TAG_INPUT_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &result, &stat);
      while (result) { // Wait until all connections are set up properly
        int64_t channel;
        MPI_Recv(&channel, 1, MPI_INT64, MPI_ANY_SOURCE,
                 MPI_TAG_INPUT_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &stat);
        assert((int)channel < N_CORRELATE_NODES);               
        assert(!connection[channel]);
        connection[channel] = true;
        n_nodes_initialised++;

        MPI_Iprobe(MPI_ANY_SOURCE, MPI_TAG_INPUT_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &result, &stat);
      }
    }
    while (n_nodes_initialised < N_CORRELATE_NODES) {
      MPI_Status stat;
      int64_t channel;
      MPI_Recv(&channel, 1, MPI_INT64, MPI_ANY_SOURCE,
               MPI_TAG_INPUT_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &stat);
      assert(!connection[channel]);
      connection[channel] = true;
      n_nodes_initialised++;
    }
  }

  // Initialise input nodes
  for (int i=0; i<N_INPUT_NODES; i++) {
#ifndef READ_DATA_FOR_CORRELATE_NODES_FROM_FILE
    // starting an input reader
    MPI_Send(&i, 1, MPI_INT32, 
             i+START_INPUT_NODES, MPI_TAG_SET_INPUT_NODE, MPI_COMM_WORLD);

    // Send the necessary control parameters:
    MPI_Transfer mpi_transfer;
    mpi_transfer.send_general_parameters(i+START_INPUT_NODES, RunPrms, GenPrms, StaPrms);

    int msg;
    MPI_Recv(&msg, 1, MPI_INT32, 
             i+START_INPUT_NODES, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD,
             &status);

    char *filename = StaPrms[i].get_mk4file();
    // strlen+1 so that \0 gets transmitted as well
    MPI_Send(filename, strlen(filename)+1, MPI_CHAR, 
             i+START_INPUT_NODES, MPI_TAG_SET_DATA_READER_FILE, MPI_COMM_WORLD);

    { // Wait for the connection to be established
      int64_t msg;
      MPI_Recv(&msg, 1, MPI_INT64, MPI_ANY_SOURCE,
               MPI_TAG_INPUT_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &status);
      assert(status.MPI_SOURCE == i+START_INPUT_NODES);
    }
#endif

    // Add correlator nodes to the input readers:
    for (int j=0; j<N_CORRELATE_NODES; j++) {

#ifdef READ_DATA_FOR_CORRELATE_NODES_FROM_FILE
      char filename[strlen(StaPrms[i].get_mk4file())+1];
      sprintf(filename, "%c%s", (char)i, StaPrms[i].get_mk4file());
      // strlen+1 so that \0 gets transmitted as well
      MPI_Send(filename, strlen(filename+1)+2, MPI_CHAR, 
               j+START_CORRELATE_NODES, MPI_TAG_ADD_DATA_READER_FILE, MPI_COMM_WORLD);
      // Wait until the connection is set up:
      int64_t msg;
      MPI_Recv(&msg, 1, MPI_INT64, MPI_ANY_SOURCE,
               MPI_TAG_INPUT_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &status);
      assert(status.MPI_SOURCE == j+START_CORRELATE_NODES);               
#else
      int32_t ranks[3] = {i, j, j+START_CORRELATE_NODES};
      MPI_Send(ranks, 3, MPI_INT32, 
               i + START_INPUT_NODES,
               MPI_TAG_ADD_OUTPUT_CONNECTION_MULTIPLE_INPUT_TCP, MPI_COMM_WORLD);
      // Wait until the connection is set up:
      int64_t msg;
      MPI_Recv(&msg, 1, MPI_INT64, MPI_ANY_SOURCE,
               MPI_TAG_INPUT_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &status);
      assert(status.MPI_SOURCE == i + START_INPUT_NODES);               
#endif
    }
  }

  if (err != 0) return;
  
  // set manager_controller:
  get_log_writer().MPI(1,"Initialisation ready");
}

void Manager_node::start() {
#ifndef READ_DATA_FOR_CORRELATE_NODES_FROM_FILE
  for (int input_node=0; input_node<GenPrms.get_nstations(); input_node++) {
    for (int corr_node=0; corr_node<N_CORRELATE_NODES; corr_node++) {
      // Stream, start time, stop time
      int64_t msg[3] = {corr_node, 0, 0};
      MPI_Send(&msg, 3, MPI_INT64, input_node+START_INPUT_NODES,
               MPI_TAG_INPUT_NODE_INPUT_STREAM_SET_PRIORITY, MPI_COMM_WORLD);
    }
  }
#endif
  
  
  int last_correlator_node = -1;
  while (GenPrms.get_duration() > 0) {
    // Check for MPI messages
    int result;
    while ((result = check_and_process_waiting_message()) != NO_MESSAGE) {
      if (result == TERMINATE_NODE) {
        return;
      }
    }
    
    // Check if there are Correlate nodes waiting:
    bool searching = true;
    
    // Duration of one time slice in seconds:
    int slice_duration = 1;
    for (int i=0; (i<numtasks) && (GenPrms.get_duration()>0); i++) {
      if ((state_correlate_nodes[i] == READY) && (last_correlator_node != i)) {
        searching = false;
        last_correlator_node = i;

        int64_t times[] = {slicenr,
                         GenPrms.get_usStart(),
                         min(GenPrms.get_duration(), slice_duration)};
        MPI_Send(times, 3, MPI_INT64, i,
                 MPI_TAG_CORRELATE_TIME_SLICE, MPI_COMM_WORLD);
        get_log_writer()(0) << "Start a new time slice "
                            << " sliceNr: " << times[0]
                            << ", corr.node: " << i
                            << ", start: " << times[1]
                            << ", dur: " << times[2] 
                            << std::endl;
        // Only one time slice for now
        GenPrms.set_duration(GenPrms.get_duration()-times[2]);
        GenPrms.set_usStart(GenPrms.get_usStart()+times[2]*1000000);

        slicenr ++;

        // The node is now correlating:
        state_correlate_nodes[i] = CORRELATING;
      }
    }
    
    if (searching) {
      // Nothing will happen until the next MPI message arrives: 
      check_and_process_message();
    }    
  }
  
  // Wait untill all correlation nodes are finished
  bool finished;
  do {
    finished = true; 
    for (int i=0; (i<numtasks); i++) {
      if (state_correlate_nodes[i] == INITIALISING) {
        // The node was no correlate node, no need to terminate
        state_correlate_nodes[i] = FINISHED;
      } else if (state_correlate_nodes[i] == READY) {
        // Terminate the correlate node
        int type = 0;
        MPI_Send(&type, 1, MPI_INT32, 
                 i, MPI_TAG_END_NODE, MPI_COMM_WORLD);
        state_correlate_nodes[i] = FINISHED;
      }
      
      finished &= (state_correlate_nodes[i] == FINISHED);
    }
    if (!finished) {
      // Nothing will happen until the next MPI message arrives: 
      check_and_process_message();
    }
  } while (!finished);
  
  // Terminate the input nodes
  int type = 0;
  for (int i=0; i<N_INPUT_NODES; i++) {
    MPI_Send(&type, 1, MPI_INT32, i+START_INPUT_NODES, 
             MPI_TAG_END_NODE, MPI_COMM_WORLD);
  }

  // Terminate the output node  
  MPI_Send(&slicenr, 1, MPI_INT32, RANK_OUTPUT_NODE, 
           MPI_TAG_OUTPUT_NODE_CORRELATION_READY, MPI_COMM_WORLD);
}

int Manager_node::read_control_file(char *control_file) {
  if (initialise_control(control_file, get_log_writer(),
                         RunPrms, GenPrms, StaPrms) != 0) {
    get_log_writer().error("Initialisation using control file failed");
    return 1;
  }
  return 0;
}

int Manager_node::send_control_parameters_to_controller_node(int node) {
  // Send the filename of the data files directly to the correlator nodes:
//  // strlen+1 so that \0 gets transmitted as well
//  for (int i=0; i<GenPrms.get_nstations(); i++) {
//    char *infile_data = StaPrms[i].get_mk4file();
//    MPI_Send(infile_data, strlen(infile_data)+1, MPI_CHAR, node,
//             MPI_TAG_SET_INPUT_NODE_FILE, MPI_COMM_WORLD);
//  }

  // Send the necessary control parameters:
  MPI_Transfer mpi_transfer;
  mpi_transfer.send_general_parameters(node, RunPrms, GenPrms, StaPrms);

  // Send the delay tables:
  for (int sn=0; sn<GenPrms.get_nstations(); sn++) {
    DelayTable delay; 
    get_log_writer() << StaPrms[sn].get_delaytable() << std::endl;
    delay.set_cmr(GenPrms);
    int retval = delay.readDelayTable(StaPrms[sn].get_delaytable());
    if (retval != 0) {
      get_log_writer().error("error while reading delay table.");
      return retval;
    }
    mpi_transfer.send_delay_table(delay, sn, node);
  }

  // Send the name of the correlator product file:
//  const char *outfile_data = GenPrms.get_corfile();
//  MPI_Send((void *)outfile_data, strlen(outfile_data)+1, MPI_CHAR, node,
//           MPI_TAG_SET_OUTPUT_NODE_FILE, MPI_COMM_WORLD);
           
  return 0;
}


void Manager_node::hook_added_data_reader(size_t reader) {
}

void Manager_node::hook_added_data_writer(size_t writer) {
}

void Manager_node::set_correlating_state(int node, CORRELATING_STATE state) {
  assert ((node >= 0) && (((size_t)node) < state_correlate_nodes.size()));
  state_correlate_nodes[node] = state;
}
