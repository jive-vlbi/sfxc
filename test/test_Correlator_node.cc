/* Author(s): Nico Kruithof, 2007
 * 
 * $Id$
 */

#include <types.h>
#include <sfxc_mpi.h>
#include <Correlator_node.h>
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
/// TODO: NGHK: REMOVE THESE <------------ UNTIL HERE

#include <MPI_Transfer.h>
#include <Log_writer_cout.h>
#include <Log_writer_void.h>

int main(int argc, char *argv[]) {
  // MPI
  int numtasks, rank;

  //initialisation
  int status = MPI_Init(&argc,&argv);
  if (status != MPI_SUCCESS) {
    std::cout << "Error starting MPI program. Terminating.\n";
    MPI_Abort(MPI_COMM_WORLD, status);
    return 1;
  }

  // get the number of tasks set at commandline (= number of processors)
  MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
  // get the ID (rank) of the task, fist rank=0, second rank=1 etc.
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  assert(numtasks == 2);

  Log_writer_void log_writer(0);
  set_log_writer(log_writer);

  ///////////////////////////
  //  The real work
  ///////////////////////////
  
  int correlator_node = 1;
  assert(correlator_node+RANK_MANAGER_NODE == 1);
  
  if (rank == correlator_node) {
    Correlator_node correlator_node(rank);
    correlator_node.start();
  } else {
    Log_writer_void log_writer(0);
    // Initialise correlator node
    assert(argc==2);
    char *control_file = argv[1];
    //"/jop35_0/kruithof/data/n05c2/sfxc_n06c2_McNtTrWb.nodel.ctrl";
    //char *control_file = "/jop35_0/kruithof/data/n05c2/sfxc_n06c2_WbWb.nodel.ctrl";
    if (initialise_control(control_file, log_writer) != 0) {
      log_writer(0) << "Initialisation using control file failed" << std::endl;
      return 1;
    }

    // strlen+1 so that \0 gets transmitted as well
    for (int i=0; i<GenPrms.get_nstations(); i++) {
      char *infile_data = StaPrms[i].get_mk4file();
      MPI_Send(infile_data, strlen(infile_data)+1, MPI_CHAR, correlator_node,
               MPI_TAG_SET_DATA_READER_FILE, MPI_COMM_WORLD);
    }

    MPI_Transfer mpi_transfer;
    mpi_transfer.send_general_parameters(correlator_node);

    for (int i=0; i<GenPrms.get_nstations(); i++) {
      DelayTable delay; 
      log_writer << StaPrms[i].get_delaytable() << std::endl;
      int retval = delay.readDelayTable(StaPrms[i].get_delaytable(),
                                        BufTime );
      if (retval != 0) {
        log_writer << "ERROR: when reading delay table.\n";
        return retval;
      }
      mpi_transfer.send_delay_table(delay, 1);
    }

    const char *outfile_data = GenPrms.get_corfile();
    MPI_Send((void *)outfile_data, strlen(outfile_data)+1, MPI_CHAR, correlator_node,
             MPI_TAG_SET_DATA_WRITER_FILE, MPI_COMM_WORLD);
    
//    int start_time[] = {GenPrms.get_yst(),
//                        GenPrms.get_dst(),
//                        GenPrms.get_hst(),
//                        GenPrms.get_mst(),
//                        GenPrms.get_sst()};
//    MPI_Send(start_time, 5, MPI_INT32, 1,
//             MPI_TAG_SET_START_TIME, MPI_COMM_WORLD);
//   
//    int stop_time[] = {GenPrms.get_ysp(),
//                       GenPrms.get_dsp(),
//                       GenPrms.get_hsp(),
//                       GenPrms.get_msp(),
//                       GenPrms.get_ssp()};
//    MPI_Send(stop_time, 5, MPI_INT32, 1,
//             MPI_TAG_SET_STOP_TIME, MPI_COMM_WORLD);
    INT64 times[] = {GenPrms.get_usStart(), GenPrms.get_usStop()};
    MPI_Send(times, 2, MPI_INT64, correlator_node,
             MPI_TAG_SET_TIME_SLICE, MPI_COMM_WORLD);

    int cmd = 0;
    MPI_Send(&cmd, 1, MPI_INT32, correlator_node,
             MPI_TAG_START_CORRELATE_NODE, MPI_COMM_WORLD);

    bool finished = false;
    MPI_Status status, status2;
    while (!finished) {
      MPI_Probe(correlator_node, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      assert(status.MPI_SOURCE == RANK_MANAGER_NODE);

      switch (status.MPI_TAG) {
        case MPI_TAG_CORRELATE_ENDED:
        {
          log_writer << "MPI_TAG_CORRELATE_ENDED " << std::endl;
          // Wait for data node to finish
          int i=0;
          MPI_Recv(&i, 1, MPI_INT32, status.MPI_SOURCE,
                   status.MPI_TAG, MPI_COMM_WORLD, &status2);
  
          // Terminate data node
          MPI_Send(&i, 1, MPI_INT32, correlator_node,
                   MPI_TAG_CORRELATION_READY, MPI_COMM_WORLD);
          finished = true;
          break;
        }
        case MPI_TAG_TEXT_MESSAGE:
        {
          int size;
          MPI_Get_elements(&status, MPI_CHAR, &size);
          assert(size > 0);
          char message[size];
          MPI_Recv(&message, size, MPI_CHAR, status.MPI_SOURCE,
                   status.MPI_TAG, MPI_COMM_WORLD, &status2);

          log_writer << "MPI_TAG_TEXT_MESSAGE: " << message << std::endl;
          break;            
        }
      }      
      log_writer << std::endl << __LINE__ << " HERE" << std::endl;
    }
  }



  //close the mpi stuff
  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Finalize();

  return 0;
}
