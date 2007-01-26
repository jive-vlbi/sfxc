/*
CVS keywords
$Author$
$Date$
$Name$
$Revision$
$Source$
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
    log_controller(log_writer_cout), 
    manager_controller(*this)

{
  assert(rank == 0);

  add_controller(&log_controller);
  add_controller(&manager_controller);
  
  int err;
  err = read_control_file(control_file);
  if (err != 0) return;

  // 1 manager node, 1 output node, Nstations input nodes 
  // and at least 1 correlator node
  assert(Nstations+3 <= numtasks);
  log_writer(0) << "nStations: " << GenPrms.get_nstations() << std::endl;
  Ncorrelator_nodes = numtasks - (Nstations+2); 

  
  {
    // Node 1 is the output node
    const char *filename = GenPrms.get_corfile();
    MPI_Send((void *)filename, strlen(filename)+1, MPI_CHAR, 1, 
             MPI_TAG_SET_OUTPUT_NODE_FILE, MPI_COMM_WORLD);
  }  

  // Nstations+2 and later are correlate nodes
  for (int i=Nstations+2; i<numtasks; i++) {
    // starting a correlator node
    int type = MPI_TAG_SET_CORRELATOR_NODE;
    MPI_Send(&type, 1, MPI_INT, i, 
             MPI_TAG_SET_CORRELATOR_NODE, MPI_COMM_WORLD);
    err = send_control_parameters_to_controller_node(i);
    if (err != 0) return;
    
    // Notify the output node (node 1):
//    MPI_Send(&i, 1, MPI_INT, 
//             1, MPI_TAG_CREATE_OUTPUT_STREAM_TCP, MPI_COMM_WORLD);
  }

  // Node 2 to Nstations+1 are input nodes
  for (int i=0; i<Nstations; i++) {
    // starting an input reader
    char *filename = StaPrms[i].get_mk4file();
    // strlen+1 so that \0 gets transmitted as well
    MPI_Send(filename, strlen(filename)+1, MPI_CHAR, 
             i+2, MPI_TAG_SET_INPUT_NODE_FILE, MPI_COMM_WORLD);

    // Add correlator nodes to the input readers:
//    for (int j=Nstations+2; j<numtasks; j++) {
//      MPI_Send(&j, 1, MPI_INT, i+2, 
//               MPI_TAG_ADD_CORRELATOR_NODE, MPI_COMM_WORLD);
//    }
  }
  if (err != 0) return;
  
  // set manager_controller:
  set_start_time(GenPrms.get_usStart());
  set_stop_time(GenPrms.get_usStop());
  
  log_writer.MPI(1,"Initialisation ready");
}

void Manager_node::start() {
  Node::start();

  int type = 0;

  // End program:
  for (int i=0; i<GenPrms.get_nstations(); i++) {
    MPI_Send(&type, 1, MPI_INT, i+2, MPI_TAG_CORRELATION_READY, MPI_COMM_WORLD);
  }
  
    MPI_Send(&type, 1, MPI_INT, 1, MPI_TAG_CORRELATION_READY, MPI_COMM_WORLD);
}

int Manager_node::read_control_file(char *control_file) {
  if (initialise_control(control_file, log_writer) != 0) {
    log_writer.error("Initialisation using control file failed");
    return 1;
  }
  Nstations = GenPrms.get_nstations();
  return 0;
}

int Manager_node::send_control_parameters_to_controller_node(int node) {
  // strlen+1 so that \0 gets transmitted as well
  for (int i=0; i<GenPrms.get_nstations(); i++) {
    char *infile_data = StaPrms[i].get_mk4file();
    MPI_Send(infile_data, strlen(infile_data)+1, MPI_CHAR, node,
             MPI_TAG_SET_INPUT_NODE_FILE, MPI_COMM_WORLD);
  }

  MPI_Transfer mpi_transfer;
  mpi_transfer.send_general_parameters(node);

  for (int i=0; i<GenPrms.get_nstations(); i++) {
    DelayTable delay; 
    log_writer << StaPrms[i].get_delaytable() << std::endl;
    int retval = delay.readDelayTable(StaPrms[i].get_delaytable(),
                                      BufTime );
    if (retval != 0) {
      log_writer.error("error while reading delay table.");
      return retval;
    }
    mpi_transfer.send_delay_table(delay, node);
  }

  const char *outfile_data = GenPrms.get_corfile();
  MPI_Send((void *)outfile_data, strlen(outfile_data)+1, MPI_CHAR, node,
           MPI_TAG_SET_OUTPUT_NODE_FILE, MPI_COMM_WORLD);
           
  return 0;
}

