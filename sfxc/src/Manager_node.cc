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
  : Node(rank), numtasks(numtasks)
{
  assert(rank == 0);
  
  int err;
  err = read_control_file(control_file);
  if (err != 0) return;
  
  assert(Nstations+2 <= numtasks);

  for (int i=Nstations+1; i<numtasks; i++) {
    // starting a correlator node
    int type = MPI_TAG_SET_CORRELATOR_NODE;
    MPI_Send(&type, 1, MPI_INT, i, 
             MPI_TAG_SET_CORRELATOR_NODE, MPI_COMM_WORLD);
    err = send_control_parameters_to_controller_node(i);
    if (err != 0) return;
  }

  for (int i=1; i<=Nstations; i++) {
    // starting an input reader
    char *filename = StaPrms[i-1].get_mk4file();
    // strlen+1 so that \0 gets transmitted as well
    MPI_Send(filename, strlen(filename)+1, MPI_CHAR, i,
             MPI_TAG_SET_INPUT_NODE_FILE, MPI_COMM_WORLD);

    // Add correlator nodes to the input readers:
    for (int j=Nstations+1; j<numtasks; j++) {
      MPI_Send(&j, 1, MPI_INT, i, 
               MPI_TAG_ADD_CORRELATOR_NODE, MPI_COMM_WORLD);
    }
  }
  if (err != 0) return;
  log_writer.MPI(1,"Initialisation ready");
}

void Manager_node::start() {
  Node::start();

  // End program:
  for (int i=1; i<numtasks; i++) {
    int type = MPI_MSG_CORRELATION_READY;
    MPI_Send(&type, 1, MPI_INT, i, MPI_TAG_COMMUNICATION, MPI_COMM_WORLD);
  }
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

