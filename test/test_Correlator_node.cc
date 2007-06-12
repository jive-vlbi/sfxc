/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <Correlator_node.h>
#include <Log_node.h>

#include <types.h>
#include <utils.h>

/// TODO: NGHK: REMOVE THESE: <------------ FROM HERE
#include "constPrms.h"
#include "runPrms.h"
#include "genPrms.h"
#include "staPrms.h"
#include "genFunctions.h"
#include "InData.h"
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

void 
send_control_parameters_to_controller_node(char *filename,
                                           int rank, 
                                           Log_writer &log_writer) {
  if (initialise_control(filename, log_writer, RunPrms, GenPrms, StaPrms)
      != 0) {
    log_writer(0) << "Initialisation using control file \'" << filename 
                  << "\'failed" << std::endl;
    return;
  }

  assert(GenPrms.get_nstations() <= 255);
  for (int i=0; i<GenPrms.get_nstations(); i++) {
    int length = strlen(StaPrms[i].get_mk4file())+1;
    char msg[length];
    sprintf(msg, "%c%s", (char)i, StaPrms[i].get_mk4file()); 
    // strlen+1 so that \0 gets transmitted as well
    MPI_Send(msg, length+1, MPI_CHAR, rank,
             MPI_TAG_ADD_DATA_READER_FILE, MPI_COMM_WORLD);
  }

  MPI_Transfer mpi_transfer;
  mpi_transfer.send_general_parameters(rank);

  for (int sn=0; sn<GenPrms.get_nstations(); sn++) {
    DelayTable delay; 
    log_writer << StaPrms[sn].get_delaytable() << std::endl;
    int retval = delay.readDelayTable(StaPrms[sn].get_delaytable());
    if (retval != 0) {
      log_writer << "ERROR: when reading delay table.\n";
      return;
    }
    mpi_transfer.send_delay_table(delay, sn, rank);
  }

  const char *outfile_data = GenPrms.get_corfile();
  MPI_Send((void *)outfile_data, strlen(outfile_data)+1, MPI_CHAR, rank,
           MPI_TAG_SET_DATA_WRITER_FILE, MPI_COMM_WORLD);

}


int main(int argc, char *argv[]) {
  // MPI
  int numtasks, rank;

  //initialisation
  int error = MPI_Init(&argc,&argv);
  if (error != MPI_SUCCESS) {
    std::cout << "Error starting MPI program. Terminating.\n";
    MPI_Abort(MPI_COMM_WORLD, error);
    return 1;
  }

  // get the number of tasks set at commandline (= number of processors)
  MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
  // get the ID (rank) of the task, fist rank=0, second rank=1 etc.
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  assert(numtasks == 3);

  ///////////////////////////
  //  The real work
  ///////////////////////////
  
  int rank_correlator_node = 2;
  assert(rank_correlator_node+RANK_MANAGER_NODE+RANK_LOG_NODE == 3);
  
  if (rank == RANK_MANAGER_NODE) {
    MPI_Status status, status2;
    { // Initialise the log node
      int msg=0;
      MPI_Send(&msg, 1, MPI_INT32, 
               RANK_LOG_NODE, MPI_TAG_SET_LOG_NODE, MPI_COMM_WORLD);
      MPI_Send(&msg, 1, MPI_INT32, 
               RANK_LOG_NODE, MPI_TAG_LOG_NODE_SET_OUTPUT_COUT, MPI_COMM_WORLD);
      MPI_Recv(&msg, 1, MPI_INT32, 
               RANK_LOG_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD, &status);
    }  

    { // Initialise the correlator node
      int type = MPI_TAG_SET_CORRELATOR_NODE;
      MPI_Send(&type, 1, MPI_INT32, rank_correlator_node, 
               MPI_TAG_SET_CORRELATOR_NODE, MPI_COMM_WORLD);
      int msg;
      MPI_Recv(&msg, 1, MPI_INT32, 
               rank_correlator_node, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD,
               &status);
    }

    Log_writer_mpi log_writer(RANK_MANAGER_NODE);
    // Send the control parameters to the correlator node:
    assert(argc==2);
    send_control_parameters_to_controller_node(argv[1], 
                                               rank_correlator_node, 
                                               log_writer);

    { // Start a single time slice
      INT32 i;
      MPI_Recv(&i, 1, MPI_INT32, rank_correlator_node,
               MPI_TAG_CORRELATE_ENDED, MPI_COMM_WORLD, &status2);
  
      INT64 times[] = {0, // Slice number
                       GenPrms.get_usStart(),
                       GenPrms.get_duration()};
      MPI_Send(times, 3, MPI_INT64, rank_correlator_node,
               MPI_TAG_CORRELATE_TIME_SLICE, MPI_COMM_WORLD);
    }

    bool finished = false;
    while (!finished) {
      MPI_Probe(rank_correlator_node, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      //assert(status.MPI_SOURCE == RANK_MANAGER_NODE);

      switch (status.MPI_TAG) {
        case MPI_TAG_CORRELATE_ENDED:
        {
          log_writer.MPI(2, print_MPI_TAG(status.MPI_TAG));
          // Wait for data node to finish
          INT32 i=0;
          MPI_Recv(&i, 1, MPI_INT32, status.MPI_SOURCE,
                   MPI_TAG_CORRELATE_ENDED, MPI_COMM_WORLD, &status2);
  
          // Terminate data node
          MPI_Send(&i, 1, MPI_INT32, rank_correlator_node,
                   MPI_TAG_CORRELATION_READY, MPI_COMM_WORLD);
          finished = true;
          break;
        }
        case MPI_TAG_TEXT_MESSAGE:
        {
          log_writer.MPI(2, print_MPI_TAG(status.MPI_TAG));
          int size;
          MPI_Get_elements(&status, MPI_CHAR, &size);
          assert(size > 0);
          char message[size];
          MPI_Recv(&message, size, MPI_CHAR, status.MPI_SOURCE,
                   status.MPI_TAG, MPI_COMM_WORLD, &status2);

          log_writer << "MPI_TAG_TEXT_MESSAGE: " << message << std::endl;
          break;            
        }
        default: {
          char err[80];
          snprintf(err, 80, "Unknown event %s", print_MPI_TAG(status.MPI_TAG));
          log_writer.error(err);

          // Remove event:  
          int size;
          MPI_Get_elements(&status, MPI_CHAR, &size);
          assert(size >= 0);
          char msg[size];
          MPI_Status status2;
          MPI_Recv(&msg, size, MPI_CHAR, status.MPI_SOURCE,
                   status.MPI_TAG, MPI_COMM_WORLD, &status2);
        }
      }
    }
    {
      MPI_Send(&rank, 1, MPI_INT, 
               RANK_LOG_NODE, MPI_TAG_LOG_MESSAGES_ENDED, MPI_COMM_WORLD);
    }
  } else {
    MPI_Status status;
    INT32 msg;
    MPI_Recv(&msg, 1, MPI_INT32, 
             RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    switch (status.MPI_TAG) {
    case MPI_TAG_SET_CORRELATOR_NODE: 
      {
        assert (rank == rank_correlator_node);
        Correlator_node correlator(rank, numtasks, 10);
        correlator.start();
        break;
      }
    case MPI_TAG_SET_LOG_NODE: 
      {
        assert (RANK_LOG_NODE == rank);
        int numtasks;
        MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
        Log_node log_node(rank,numtasks);
        log_node.start();
        break;
      }
//    case MPI_TAG_SET_OUTPUT_NODE: 
//      {
//        assert (RANK_OUTPUT_NODE == rank);
//        int numtasks;
//        MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
//        Output_node output_node(rank,numtasks);
//        output_node.start();
//        break;
//      }
    default:
      {
        std::cout << "Unknown node type " << status.MPI_TAG << std::endl;
        assert(false);
        return 1;
      }
    }
  }



  //close the mpi stuff
  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Finalize();

  return 0;
}
