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
uint32_t seed;
/// TODO: NGHK: REMOVE THESE <------------ UNTIL HERE

#include <MPI_Transfer.h>
#include <Log_writer_cout.h>
#include <Log_writer_void.h>

void initialise_correlator_node(int rank_correlator_node, 
                                char *ctrl_file,
                                Log_writer &log_writer) {
  initialise_control(ctrl_file, log_writer, RunPrms, GenPrms, StaPrms);

  MPI_Status status;

  { // Initialise the correlator node
    int type = MPI_TAG_SET_CORRELATOR_NODE;
    MPI_Send(&type, 1, MPI_INT32, rank_correlator_node, 
             MPI_TAG_SET_CORRELATOR_NODE, MPI_COMM_WORLD);
    int msg;
    MPI_Recv(&msg, 1, MPI_INT32, 
             rank_correlator_node, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD,
             &status);

    // Transfer the control parameters
    MPI_Transfer mpi_transfer;
    mpi_transfer.send_general_parameters(rank_correlator_node, 
                                         RunPrms, GenPrms, StaPrms);

    // Transfer the delay tables
    for (int station_nr = 0; 
         station_nr < GenPrms.get_nstations(); 
         station_nr++) {
      DelayTable delay; 
      delay.set_cmr(GenPrms);
      int retval = delay.readDelayTable(StaPrms[station_nr].get_delaytable());
      if (retval != 0) {
        get_log_writer().error("error while reading delay table.");
        return;
      }
      mpi_transfer.send_delay_table(delay, station_nr, rank_correlator_node);
    }
  }
  { // Set the data readers and writer
    int64_t return_msg;
    for (int station_nr = 0; 
         station_nr < GenPrms.get_nstations(); 
         station_nr++) {
      int length = strlen(StaPrms[station_nr].get_mk4file())+2;
      char filename[length];
      snprintf(filename, length, "%c%s", 
               (char)station_nr, StaPrms[station_nr].get_mk4file());
      // strlen+1 so that \0 gets transmitted as well
      MPI_Send(filename, length, MPI_CHAR, 
               rank_correlator_node, 
               MPI_TAG_ADD_DATA_READER_FILE, 
               MPI_COMM_WORLD);
    
      MPI_Recv(&return_msg, 1, MPI_INT64, 
               rank_correlator_node, MPI_TAG_INPUT_CONNECTION_ESTABLISHED, 
               MPI_COMM_WORLD, &status);
    }

    const char *filename = GenPrms.get_corfile();
    MPI_Send((void *)filename, strlen(filename)+1, MPI_CHAR, 
             rank_correlator_node, MPI_TAG_SET_DATA_WRITER_FILE, MPI_COMM_WORLD);
    MPI_Recv(&return_msg, 1, MPI_INT64, 
             rank_correlator_node, MPI_TAG_INPUT_CONNECTION_ESTABLISHED, 
             MPI_COMM_WORLD, &status);
  }
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

  DEBUG_MSG(" pid = " << getpid());

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
               RANK_LOG_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD, 
               &status);
    }  

    Log_writer_mpi log_writer(RANK_MANAGER_NODE);
    initialise_correlator_node(rank_correlator_node, 
                               argv[1],
                               log_writer);

    { // Start a single time slice
      int32_t i;
      MPI_Recv(&i, 1, MPI_INT32, rank_correlator_node,
               MPI_TAG_CORRELATION_OF_TIME_SLICE_ENDED, MPI_COMM_WORLD,
               &status2);

      int64_t times[] = {0, // Slice number
                         GenPrms.get_usStart(),
                         GenPrms.get_duration()};
      MPI_Send(times, 3, MPI_INT64, rank_correlator_node,
               MPI_TAG_CORRELATE_TIME_SLICE, MPI_COMM_WORLD);
    }

    bool finished = false;
    while (!finished) {
      MPI_Probe(rank_correlator_node, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      switch (status.MPI_TAG) {
      case MPI_TAG_CORRELATION_OF_TIME_SLICE_ENDED:
        {
          log_writer.MPI(2, print_MPI_TAG(status.MPI_TAG));

          // Terminate data node
          int32_t i=0;
          MPI_Send(&i, 1, MPI_INT32, rank_correlator_node,
                   MPI_TAG_END_NODE, MPI_COMM_WORLD);
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
    int32_t msg;
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
