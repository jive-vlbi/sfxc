/*
  $Author$
  $Date$
  $Name$
  $Revision$
  $Source$
  
  Tests reading a file from disk and then writing it back using a Data_node
*/

#include <types.h>
#include <Input_node.h>
#include <fstream>
#include <assert.h>

#include <stdio.h>
#include <iostream>

/// TODO: NGHK: REMOVE THESE: <------------ FROM HERE
#include "constPrms.h"
#include "runPrms.h"
#include "genPrms.h"
#include "staPrms.h"
#include "genFunctions.h"
#include "InData.h"
#include "ProcessData.h"
#include "delayTable.h"
//global variables
//declaration and default settings run parameters
RunP RunPrms;
//declaration and default settings general parameters
GenP GenPrms;
//station parameters class, declaration and default settings
StaP StaPrms[NstationsMax];
// used for randomising numbers for Headers in Mk4 file
UINT32 seed;
//declarations for offsets
INT64 sliceStartByte[NstationsMax][NprocessesMax];
INT64 sliceStartTime [NprocessesMax];
INT64 sliceStopTime  [NprocessesMax];
INT64 sliceTime;
/// TODO: NGHK: REMOVE THESE <------------ UNTIL HERE


int main(int argc, char *argv[]) {
  // MPI
  int numtasks, rank;

  //initialisation
  int status = MPI_Init(&argc,&argv);
  if (status != MPI_SUCCESS) {
    std::cout << "Error starting MPI program. Terminating.\n";
    MPI_Abort(MPI_COMM_WORLD, status);
  }

  // get the number of tasks set at commandline (= number of processors)
  MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
  // get the ID (rank) of the task, fist rank=0, second rank=1 etc.
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  ///////////////////////////
  //  The real work
  ///////////////////////////
  assert(argc == 3);
  char *infile = argv[1];
  char *outfile = argv[2];

  if (rank == 0) {
    // Initialise data node
        
    // strlen+1 so that \0 gets transmitted as well
    MPI_Send(infile, strlen(infile)+1, MPI_CHAR, 1,
             MPI_TAG_SET_INPUT_NODE_FILE, MPI_COMM_WORLD);

    MPI_Send(outfile, strlen(outfile)+1, MPI_CHAR, 1,
             MPI_TAG_SET_OUTPUT_NODE_FILE, MPI_COMM_WORLD);

    MPI_Status status;
    MPI_Probe(1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    assert(status.MPI_TAG == MPI_TAG_DATASTREAM_EMPTY);
    assert(status.MPI_SOURCE == 1);
  
  
    // Wait for data node to finish
    int i;
    MPI_Status status2;
    MPI_Recv(&i, 1, MPI_INT, MPI_ANY_SOURCE,
             MPI_TAG_DATASTREAM_EMPTY, MPI_COMM_WORLD, &status2);
   
   
    // Terminate data node
    i=0;
    MPI_Send(&i, 1, MPI_INT, 1,
             MPI_TAG_CORRELATION_READY, MPI_COMM_WORLD);

  } else if (rank == 1) {
    Input_node input_node(rank);
    input_node.start();
  }



  //close the mpi stuff
  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Finalize();

  return 0;
}
