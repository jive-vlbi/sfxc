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
    manager_controller(*this), start_time(-1), duration(-1)

{
  assert(rank == 0);

  add_controller(&manager_controller);
  
  int err;
  err = read_control_file(control_file);
  if (err != 0) return;

  // 1 manager node, 1 output node, Nstations input nodes 
  // and at least 1 correlator node
  assert(Nstations+3 <= numtasks);
  Ncorrelator_nodes = numtasks - (Nstations+2); 

  MPI_Status status;
  
  { // Initialise the log node
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
  for (int i=Nstations+3; i<numtasks; i++) {
    // starting a correlator node
    int type = MPI_TAG_SET_CORRELATOR_NODE;
    MPI_Send(&type, 1, MPI_INT32, i, 
             MPI_TAG_SET_CORRELATOR_NODE, MPI_COMM_WORLD);
    int msg;
    MPI_Recv(&msg, 1, MPI_INT32, 
             i, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD, &status);

    err = send_control_parameters_to_controller_node(i);
    if (err != 0) return;

    
    // Notify the output node (node 1):
    INT32 ranks[2] = {i, RANK_LOG_NODE};
    MPI_Send(ranks, 2, MPI_INT32, 
             i, 
             MPI_TAG_ADD_OUTPUT_CONNECTION_MULTIPLE_INPUT_TCP, MPI_COMM_WORLD);

//    MPI_Send(&i, 1, MPI_INT32, 
//             1, MPI_TAG_CREATE_OUTPUT_STREAM_TCP, MPI_COMM_WORLD);
  }

  // Node 2 to Nstations+1 are input nodes
  for (int i=0; i<Nstations; i++) {
    int start_input_nodes = 3;
    // starting an input reader
    int msg=0;
    MPI_Send(&msg, 1, MPI_INT32, 
             i+start_input_nodes, MPI_TAG_SET_INPUT_NODE, MPI_COMM_WORLD);
    MPI_Recv(&msg, 1, MPI_INT32, 
             i+start_input_nodes, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD,
             &status);

    char *filename = StaPrms[i].get_mk4file();
    // strlen+1 so that \0 gets transmitted as well
    MPI_Send(filename, strlen(filename)+1, MPI_CHAR, 
             i+start_input_nodes, MPI_TAG_SET_DATA_READER_FILE, MPI_COMM_WORLD);

    // Add correlator nodes to the input readers:
    INT32 ranks[2] = {i+start_input_nodes, 0};
    for (int j=Nstations+start_input_nodes; j<numtasks; j++) {
      ranks[1] = j;
      MPI_Send(ranks, 2, MPI_INT32, 
               i+start_input_nodes,
               MPI_TAG_ADD_OUTPUT_CONNECTION_MULTIPLE_INPUT_TCP, MPI_COMM_WORLD);
//      MPI_Send(&j, 1, MPI_INT32, i+start_input_nodes, 
//               MPI_TAG_ADD_CORRELATOR_NODE_TCP, MPI_COMM_WORLD);
    }
  }
  if (err != 0) return;
  
  // set manager_controller:
  set_start_time(GenPrms.get_usStart());
  set_duration(GenPrms.get_duration());
  
  get_log_writer().MPI(1,"Initialisation ready");
}

void Manager_node::start() {
  Node::start();

  int type = 0;

  // End program:
  for (int i=0; i<GenPrms.get_nstations(); i++) {
    MPI_Send(&type, 1, MPI_INT32, i+2, MPI_TAG_CORRELATION_READY, MPI_COMM_WORLD);
  }
  
  MPI_Send(&type, 1, MPI_INT32, 1, MPI_TAG_CORRELATION_READY, MPI_COMM_WORLD);
}

int Manager_node::read_control_file(char *control_file) {
  if (initialise_control(control_file, get_log_writer()) != 0) {
    get_log_writer().error("Initialisation using control file failed");
    return 1;
  }
  Nstations = GenPrms.get_nstations();
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
  for (int i=0; i<GenPrms.get_nstations(); i++) {
    DelayTable delay; 
    get_log_writer() << StaPrms[i].get_delaytable() << std::endl;
    int retval = delay.readDelayTable(StaPrms[i].get_delaytable(),
                                      BufTime );
    if (retval != 0) {
      get_log_writer().error("error while reading delay table.");
      return retval;
    }
    mpi_transfer.send_delay_table(delay, node);
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
