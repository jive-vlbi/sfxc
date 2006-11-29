/*
CVS keywords
$Author$
$Date$
$Name$
$Revision$
$Source$

Author     : NGH Kruithof
StartDate  : 20061101
Last change: 20061124
*/

#include <Controller_node.h>
#include <iostream>
#include <assert.h>

// NGHK: Global variables
#include <runPrms.h>
#include <genPrms.h>
#include <staPrms.h>
#include <constPrms.h>
extern RunP RunPrms;
extern GenP GenPrms;
extern StaP StaPrms[NstationsMax];

Controller_node::Controller_node(int numtasks, int rank, char * ctrlFile) 
  : Node(rank), numtasks(numtasks)
{
  
  //parse control file for run parameters
  if (RunPrms.parse_ctrlFile(ctrlFile) != 0) {
    std::cerr << "ERROR: Control file "<< ctrlFile <<", program aborted.\n";
    return;
  }
  
  //show version information and control file info
  if (RunPrms.get_messagelvl()> 0)
    std::cout << "\nSource " << __FILE__ << " compiled at: "
         << __DATE__ << " " <<__TIME__ << "\n\n"
         << "Control file name "  <<  ctrlFile << "\n";
  
  //check control parameters, optionally show them
  if (RunPrms.check_params() != 0) {
    std::cerr << "ERROR: Run control parameter, program aborted.\n";
    return;
  }
  
  //parse control file for general parameters
  if (GenPrms.parse_ctrlFile(ctrlFile) != 0) {
    std::cerr << "ERROR: Control file "<< ctrlFile <<", program aborted.\n";
    return;
  }

  //check general control parameters, optionally show them
  if (GenPrms.check_params() != 0) {
    std::cerr << "ERROR: General control parameter, program aborted.\n";
    return;
  }
  
  //get the number of stations
  Nstations = GenPrms.get_nstations();
  
  //parse the control file for all station parameters
  for (int i=0; i<Nstations; i++)
    if (StaPrms[i].parse_ctrlFile(ctrlFile,i) != 0 ) {
      std::cerr << "ERROR: Control file "<< ctrlFile <<", program aborted.\n";
      return;
    }
    
  //check station control parameters, optionally show them
  for (int i=0; i<Nstations; i++){
    if (StaPrms[i].check_params() != 0 ) {
      std::cerr << "ERROR: Station control parameter, program aborted.\n";
      return;
    }
  }  

  // starting up the other processes:
  if (numtasks < Nstations+2) {
    // We need at least 1 control node, Nstations readers and 1 correlator.
    std::cerr << "ERROR: To few processes to correlate, at least "
              << Nstations+2 << " processes are needed" << std::endl;
    assert(numtasks >= Nstations+2);
  }

  for (int i=0; i<Nstations; i++) {
    // starting an input reader
    int type = MPI_INPUT_NODE;
    MPI_Send(&type, 1, MPI_INT, i+1, MPI_TAG_INIT, MPI_COMM_WORLD);
    char *input_str = StaPrms[i].get_mk4file();
    // strlen+1 so that \0 gets transmitted as well
    // 256 is the buffer size in Input_node:
    assert(strlen(input_str)+1 <= 256);
    MPI_Send(input_str, strlen(input_str)+1, MPI_CHAR, 
             i+1, MPI_TAG_INIT, MPI_COMM_WORLD);
  }
  for (int i=Nstations+1; i<numtasks; i++) {
    // starting a correlator node
    int type = MPI_CORRELATOR_NODE;
    MPI_Send(&type, 1, MPI_INT, i, MPI_TAG_INIT, MPI_COMM_WORLD);
    for (int j=1; j<=Nstations; j++) {
      MPI_Send(&j, 1, MPI_INT, i, MPI_TAG_ADD_INPUT_MPI, MPI_COMM_WORLD);
    }
    // Channels:
//     for (int i=1; i<Nstations; i++) {
//       MPI_Send(&i, 1, MPI_INT, i, MPI_TAG_ADD_INPUT, MPI_COMM_WORLD);
//     }
  }
}

void Controller_node::start() {
  // Start processing:
  

  // End program:
  for (int i=1; i<numtasks; i++) {
    int type = MPI_CORRELATION_READY;
    MPI_Send(&type, 1, MPI_INT, i, MPI_TAG_COMMUNICATION, MPI_COMM_WORLD);
  }
}
