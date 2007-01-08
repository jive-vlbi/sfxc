/*
  $Author$
  $Date$
  $Name$
  $Revision$
  $Source$
  
  Tests correlating two files from disk (based on local ip-address)
*/

#include <types.h>
#include <Correlate_node.h>
#include <fstream>
#include <assert.h>

#include <stdio.h>
#include <iostream>

#include <utils.h>

/// TODO: NGHK: REMOVE THESE: <------------ FROM HERE
#include "constPrms.h"
#include "runPrms.h"
#include "genPrms.h"
#include "staPrms.h"
#include "genFunctions.h"
#include "InData.h"
#include "ProcessData.h"
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

#include <MPI_Transfer.h>

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
  if (rank == 0) {
    // Initialise correlator node
    assert(argc==2);
    char *control_file = argv[1];
    //"/jop54_0/kruithof/data/n05c2/sfxc_n06c2_McNtTrWb.nodel.ctrl";
    //char *control_file = "/jop54_0/kruithof/data/n05c2/sfxc_n06c2_WbWb.nodel.ctrl";
    if (initialise_control(control_file) != 0) {
      std::cout << "Initialisation using control file failed" << std::endl;
    }

    // This one has to go:
    MPI_Send(control_file, strlen(control_file)+1, MPI_CHAR, 1,
             MPI_TAG_SET_CONTROL_FILE, MPI_COMM_WORLD);
            
    // strlen+1 so that \0 gets transmitted as well
    for (int i=0; i<GenPrms.get_nstations(); i++) {
      char *infile_data = StaPrms[i].get_mk4file();
      MPI_Send(infile_data, strlen(infile_data)+1, MPI_CHAR, 1,
               MPI_TAG_SET_INPUT_NODE_FILE, MPI_COMM_WORLD);
    }

     const char *outfile_data = GenPrms.get_corfile();
     MPI_Send((void *)outfile_data, strlen(outfile_data)+1, MPI_CHAR, 1,
              MPI_TAG_SET_OUTPUT_NODE_FILE, MPI_COMM_WORLD);

     int start_time[] = {GenPrms.get_yst(),
                         GenPrms.get_dst(),
                         GenPrms.get_hst(),
                         GenPrms.get_mst(),
                         GenPrms.get_sst()};
     MPI_Send(start_time, 5, MPI_INT, 1,
              MPI_TAG_SET_START_TIME, MPI_COMM_WORLD);

     int stop_time[] = {GenPrms.get_ysp(),
                        GenPrms.get_dsp(),
                        GenPrms.get_hsp(),
                        GenPrms.get_msp(),
                        GenPrms.get_ssp()};
     MPI_Send(stop_time, 5, MPI_INT, 1,
              MPI_TAG_SET_STOP_TIME, MPI_COMM_WORLD);

    MPI_Transfer mpi_transfer;
    mpi_transfer.send_general_parameters(1);

    int cmd = 0;
    MPI_Send(&cmd, 1, MPI_INT, 1,
             MPI_TAG_START_CORRELATE_NODE, MPI_COMM_WORLD);

    bool finished = false;
    MPI_Status status, status2;
    while (!finished) {
      MPI_Probe(1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      assert(status.MPI_SOURCE == 1);

      switch (status.MPI_TAG) {
        case MPI_MSG_CORRELATE_ENDED:
        {
          std::cout << "MPI_MSG_CORRELATE_ENDED " << std::endl;
          // Wait for data node to finish
          int i=0;
          MPI_Recv(&i, 1, MPI_INT, status.MPI_SOURCE,
                   status.MPI_TAG, MPI_COMM_WORLD, &status2);
  
          // Terminate data node
          MPI_Send(&i, 1, MPI_INT, 1,
                   MPI_MSG_CORRELATION_READY, MPI_COMM_WORLD);
          finished = true;
          break;
        }
        case MPI_MSG_TEXT_MESSAGE:
        {
          int size;
          MPI_Get_elements(&status, MPI_CHAR, &size);
          assert(size > 0);
          char message[size];
          MPI_Recv(&message, size, MPI_CHAR, status.MPI_SOURCE,
                   status.MPI_TAG, MPI_COMM_WORLD, &status2);

          std::cout << "MPI_MSG_TEXT_MESSAGE: " << message << std::endl;
          break;            
        }
      }      
      std::cout << std::endl << __LINE__ << " HERE" << std::endl;
    
    }
  } else if (rank == 1) {
    Correlate_node correlate_node(rank);
    correlate_node.start();
    std::cout << std::endl << rank << " READY" << std::endl;
  }



  //close the mpi stuff
  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Finalize();

  return 0;
}
