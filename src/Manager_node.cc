/* Author(s): Nico Kruithof, 2007
 * 
 * $Id$
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
  assert(rank == 0);

  add_controller(&manager_controller);
  
  int err;
  err = read_control_file(control_file);
  if (err != 0) return;

  // 0: Manager node
  // 1: Log node
  // 2: Output node
  START_INPUT_NODES = 3;
  N_INPUT_NODES = GenPrms.get_nstations();
  assert(N_INPUT_NODES > 0);
  
  START_CORRELATE_NODES = START_INPUT_NODES + N_INPUT_NODES;
  N_CORRELATE_NODES = numtasks - 3 - N_INPUT_NODES;
  assert(N_CORRELATE_NODES > 0);

  MPI_Status status;
  
  { // Initialise the log node
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

  { // Initialise the output node
    assert (RANK_OUTPUT_NODE == 2);
    
    int msg=0;
    const char *filename = GenPrms.get_corfile();
    MPI_Send(&msg, 1, MPI_INT32, 
             RANK_OUTPUT_NODE, MPI_TAG_SET_OUTPUT_NODE, MPI_COMM_WORLD);
    MPI_Send((void *)filename, strlen(filename)+1, MPI_CHAR, 
             RANK_OUTPUT_NODE, MPI_TAG_SET_DATA_WRITER_FILE, MPI_COMM_WORLD);
    MPI_Recv(&msg, 1, MPI_INT32, 
             RANK_OUTPUT_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD, &status);
  }  

  // Nstations+3 and later are correlate nodes
  for (int i=0; i<N_CORRELATE_NODES; i++) {
    // starting a correlator node
    int type = MPI_TAG_SET_CORRELATOR_NODE;
    MPI_Send(&type, 1, MPI_INT32, i+START_CORRELATE_NODES, 
             MPI_TAG_SET_CORRELATOR_NODE, MPI_COMM_WORLD);
    int msg;
    MPI_Recv(&msg, 1, MPI_INT32, 
             i+START_CORRELATE_NODES, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD, &status);

    err = send_control_parameters_to_controller_node(i+START_CORRELATE_NODES);
    if (err != 0) return;

    
    // Set the output node:
    INT32 ranks[2] = {i+START_CORRELATE_NODES, RANK_OUTPUT_NODE};
    MPI_Send(ranks, 2, MPI_INT32, 
             i+START_CORRELATE_NODES, 
             MPI_TAG_SET_OUTPUT_CONNECTION_MULTIPLE_INPUT_TCP, MPI_COMM_WORLD);

  }

  // set all nodes to INITIALISING:
  state_correlate_nodes.resize(numtasks, INITIALISING);

  { // Wait until all connections are set up properly
    bool connection[numtasks];
    for (int i=0; i<numtasks; i++) connection[i] = false;
    for (int i = 0; i < N_CORRELATE_NODES; i++) {
      INT64 channel;
      MPI_Recv(&channel, 1, MPI_INT64, RANK_OUTPUT_NODE,
               MPI_TAG_INPUT_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &status);
      assert((int)channel >= N_INPUT_NODES+3);               
      assert((int)channel < numtasks);               
      assert(!connection[channel]);
      connection[channel] = true;
    }
  }

  // Initialise input nodes
  for (int i=0; i<N_INPUT_NODES; i++) {
    // starting an input reader
    int msg=0;
    MPI_Send(&msg, 1, MPI_INT32, 
             i+START_INPUT_NODES, MPI_TAG_SET_INPUT_NODE, MPI_COMM_WORLD);
    MPI_Recv(&msg, 1, MPI_INT32, 
             i+START_INPUT_NODES, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD,
             &status);

    char *filename = StaPrms[i].get_mk4file();
    // strlen+1 so that \0 gets transmitted as well
    MPI_Send(filename, strlen(filename)+1, MPI_CHAR, 
             i+START_INPUT_NODES, MPI_TAG_SET_DATA_READER_FILE, MPI_COMM_WORLD);

    { // Wait for the connection to be established
      INT64 msg;
      MPI_Recv(&msg, 1, MPI_INT64, MPI_ANY_SOURCE,
               MPI_TAG_INPUT_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &status);
      assert(status.MPI_SOURCE == i+START_INPUT_NODES);
    }

    // Add correlator nodes to the input readers:
    INT32 ranks[2] = {i+START_INPUT_NODES, 0};
    for (int j=0; j<N_CORRELATE_NODES; j++) {
      ranks[1] = j+START_CORRELATE_NODES;
      MPI_Send(ranks, 2, MPI_INT32, 
               ranks[0],
               MPI_TAG_ADD_OUTPUT_CONNECTION_MULTIPLE_INPUT_TCP, MPI_COMM_WORLD);

      // Wait until the connection is set up:
      INT64 msg;
      MPI_Recv(&msg, 1, MPI_INT64, MPI_ANY_SOURCE,
               MPI_TAG_INPUT_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &status);
      assert(status.MPI_SOURCE == ranks[1]);               
    }
  }

  if (err != 0) return;
  
  // set manager_controller:
  get_log_writer().MPI(1,"Initialisation ready");
}

void Manager_node::start() {
  while (GenPrms.get_duration() > 0) {
    // Check for MPI messages
    while ((check_and_process_waiting_message() != NO_MESSAGE)) {
    }
    
    // Check if there are Correlate nodes waiting:
    bool searching = true;
    
    // Duration of one time slice:
    int slice_duration = 2;
    for (int i=0; (i<numtasks) && searching; i++) {
      if (state_correlate_nodes[i] == READY) {
        searching = false;
        get_log_writer() << " DURATION = " << GenPrms.get_duration() << std::endl;

        INT64 times[] = {slicenr,
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
                 i, MPI_TAG_CORRELATION_READY, MPI_COMM_WORLD);
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
             MPI_TAG_CORRELATION_READY, MPI_COMM_WORLD);
  }

  // Terminate the output node  
  MPI_Send(&type, 1, MPI_INT32, RANK_OUTPUT_NODE, 
           MPI_TAG_CORRELATION_READY, MPI_COMM_WORLD);
}

int Manager_node::read_control_file(char *control_file) {
  if (initialise_control(control_file, get_log_writer()) != 0) {
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
  mpi_transfer.send_general_parameters(node);

  // Send the delay tables:
  for (int sn=0; sn<GenPrms.get_nstations(); sn++) {
    DelayTable delay; 
    get_log_writer() << StaPrms[sn].get_delaytable() << std::endl;
    int retval = delay.readDelayTable(StaPrms[sn].get_delaytable(),
                                      BufTime );
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


void Manager_node::hook_added_data_reader(int reader) {
}

void Manager_node::hook_added_data_writer(int writer) {
}

void Manager_node::set_correlating_state(int node, CORRELATING_STATE state) {
  assert ((node >= 0) && (node < state_correlate_nodes.size()));
  state_correlate_nodes[node] = state;
}
