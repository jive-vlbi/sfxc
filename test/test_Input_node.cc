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
#include <Output_node.h>
#include <Log_node.h>

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

#include <Node.h>
#include <Data_reader2buffer.h>
#include <TCP_Connection.h>
#include <Buffer2data_writer.h>
#include <Data_writer.h>
#include <Data_writer_file.h>
#include <Data_reader_tcp.h>

int input_node = 2;
int output_node = 3;
// MPI
int numtasks, rank;

char *infile, *outfile;

//class Test_Output_node;
//
//class Test_Output_node_controller : public Controller {
//public:
//  Test_Output_node_controller(Test_Output_node &node);
//
//  Process_event_status process_event(MPI_Status &status);
//  
//private:
//  Test_Output_node & node;
//};
//
//class Test_Output_node : public Node {
//  typedef Buffer_element<char,131072>           value_type;
//public:
//  Test_Output_node(int rank, int size=100)
//  : Node(rank), ctrl(*this), buffer(size) {
//    add_controller(&ctrl);
//    reader2buffer.set_buffer(&buffer);
//    buffer2writer.set_buffer(&buffer);
//  }
//  ~Test_Output_node() {
//    assert(reader2buffer.get_data_reader()->eof());
//    assert(buffer.empty());
//    reader2buffer.stop();
//    buffer2writer.stop();
//  }
//
//  void set_reader(Data_reader *reader) {
//    reader2buffer.set_data_reader(reader);
//    reader2buffer.start();
//  }
//  void set_writer(Data_writer *writer) {
//    buffer2writer.set_data_writer(writer);
//    buffer2writer.start();
//  }
//
//
//  void start() {
//    MESSAGE_RESULT result;
//    bool finished = false;
//    do {
//      result = check_and_process_waiting_message();
//      if (result == NO_MESSAGE) {
//        usleep(100000); // .1 second:
//      }
//      if (reader2buffer.get_data_reader()!=NULL) {
//        finished = reader2buffer.get_data_reader()->eof() && buffer.empty();
//      }
//    } while (!finished);
//  }
//
//private:
//  Test_Output_node_controller    ctrl;
//  Semaphore_buffer<value_type>   buffer;
//  Data_reader2buffer<value_type> reader2buffer;
//  Buffer2data_writer<value_type> buffer2writer;
//};
//
//Test_Output_node_controller::Test_Output_node_controller(Test_Output_node &node)
//  : Controller(node), node(node) {
//  INT32 msg;
//  MPI_Send(&msg, 1, MPI_INT32, 
//           RANK_MANAGER_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD);
//}
//
//Test_Output_node_controller::Process_event_status 
//Test_Output_node_controller::process_event(MPI_Status &status) {
//  MPI_Status status2;
//  switch (status.MPI_TAG) {
//  case MPI_TAG_SET_DATA_WRITER_FILE:
//    {
//      get_log_writer().MPI(2, print_MPI_TAG(status.MPI_TAG));
//      int size;
//      MPI_Get_elements(&status, MPI_CHAR, &size);
//      assert(size > 0);
//      char filename[size];
//      MPI_Recv(&filename, size, MPI_CHAR, status.MPI_SOURCE,
//               status.MPI_TAG, MPI_COMM_WORLD, &status2);
//      
//      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
//      assert(status.MPI_TAG == status2.MPI_TAG);
//
//      Data_writer *data_writer = new Data_writer_file(filename);
//      node.set_writer(data_writer);
//
//      return PROCESS_EVENT_STATUS_SUCCEEDED;
//    }
//  case MPI_TAG_ADD_DATA_READER_TCP:
//    {
//      get_log_writer().MPI(2, print_MPI_TAG(status.MPI_TAG));
//
//      int size;
//      MPI_Get_elements(&status, MPI_UINT64, &size);
//      assert(size >= 3); // [ip-addr]+, port, rank
//      UINT64 ip_addr[size];
//      MPI_Recv(&ip_addr, size, MPI_UINT64, status.MPI_SOURCE,
//               status.MPI_TAG, MPI_COMM_WORLD, &status2);
//      
//      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
//      assert(status.MPI_TAG == status2.MPI_TAG);
//      
//      UINT64 port = ip_addr[size-2];
//      TCP_Connection connection(true); 
//      
//      Data_reader_tcp *data_reader = 
//        new Data_reader_tcp(ip_addr, size-2, port);
//        
//      node.set_reader(data_reader);
//
//      return PROCESS_EVENT_STATUS_SUCCEEDED;
//    }
//  case MPI_TAG_OUTPUT_STREAM_SET_PRIORITY:
//    {
//      get_log_writer().MPI(0, print_MPI_TAG(status.MPI_TAG));
//      INT64 weight[2];
//      MPI_Recv(&weight, 2, MPI_INT64, status.MPI_SOURCE,
//               status.MPI_TAG, MPI_COMM_WORLD, &status2);
//      
//      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
//      assert(status.MPI_TAG == status2.MPI_TAG);
//
//      return PROCESS_EVENT_STATUS_SUCCEEDED;
//    }
//  case MPI_TAG_OUTPUT_STREAM_TIME_SLICE_FINISHED: 
//    {
//      get_log_writer().MPI(0, print_MPI_TAG(status.MPI_TAG));
//      INT32 rank;
//      MPI_Recv(&rank, 1, MPI_INT32, status.MPI_SOURCE,
//               status.MPI_TAG, MPI_COMM_WORLD, &status2);
//      
//      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
//      assert(status.MPI_TAG == status2.MPI_TAG);
//
//      return PROCESS_EVENT_STATUS_SUCCEEDED;
//    }
//  }
//  return PROCESS_EVENT_STATUS_UNKNOWN;
//}

void test1() {
  MPI_Status status;
  
  if (rank == RANK_MANAGER_NODE) {
    int msg=0;
    // Log node:
    MPI_Send(&msg, 1, MPI_INT32, 
             RANK_LOG_NODE, MPI_TAG_SET_LOG_NODE, MPI_COMM_WORLD);
    MPI_Send(&msg, 1, MPI_INT32, 
             RANK_LOG_NODE, MPI_TAG_LOG_NODE_SET_OUTPUT_COUT, MPI_COMM_WORLD);
    MPI_Recv(&msg, 1, MPI_INT32, 
             RANK_LOG_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD, &status);
    
    // Input_node node:
    MPI_Send(&msg, 1, MPI_INT32, 
             input_node, MPI_TAG_SET_INPUT_NODE, MPI_COMM_WORLD);
    MPI_Recv(&msg, 1, MPI_INT32, 
             input_node, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD, &status);
        
    // Connect the input and output:
    // strlen+1 so that \0 gets transmitted as well

    MPI_Send(infile, strlen(infile)+1, MPI_CHAR, 
             input_node, MPI_TAG_SET_DATA_READER_FILE, MPI_COMM_WORLD);
    { // output file: 
      int size = strlen(outfile)+1+sizeof(int), position=0;
      char buffer[size];
      MPI_Pack(&output_node, 1, MPI_INT32, 
               buffer, size, &position, MPI_COMM_WORLD);
      MPI_Pack(outfile, strlen(outfile)+1, MPI_CHAR, 
               buffer, size, &position, MPI_COMM_WORLD);
    
      MPI_Send(buffer, size, MPI_PACKED, 
               input_node, MPI_TAG_ADD_DATA_WRITER_FILE, MPI_COMM_WORLD);
    }

    // set priorities:
    INT64 priority_in[] = {output_node,0,0};
    MPI_Send(&priority_in, 2, MPI_INT64, 
             input_node, MPI_TAG_INPUT_STREAM_SET_PRIORITY, MPI_COMM_WORLD);

    // Wait for data nodes to finish
    MPI_Status status;
    int i;
    MPI_Recv(&i, 1, MPI_INT32, MPI_ANY_SOURCE,
             MPI_TAG_DATASTREAM_EMPTY, MPI_COMM_WORLD, &status);
    assert(status.MPI_TAG == MPI_TAG_DATASTREAM_EMPTY);
    assert(status.MPI_SOURCE == input_node);
   
    MPI_Send(&rank, 1, MPI_INT, 
             RANK_LOG_NODE, MPI_TAG_LOG_MESSAGES_ENDED, MPI_COMM_WORLD);

  } else {
    // Don't use the output node:
    if (rank == output_node) {
      MPI_Send(&rank, 1, MPI_INT, 
               RANK_LOG_NODE, MPI_TAG_LOG_MESSAGES_ENDED, MPI_COMM_WORLD);
      return;
    }
    
    INT32 msg;
    MPI_Recv(&msg, 1, MPI_INT32, 
             RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    
    switch (status.MPI_TAG) {
      case MPI_TAG_SET_LOG_NODE: {
        assert (RANK_LOG_NODE == rank);
        Log_node log_node(rank,numtasks);
        log_node.start();
        break;
      }
      case MPI_TAG_SET_INPUT_NODE: {
        assert(rank == input_node);
        Input_node node(rank);
        node.start();
        break;
      }
      default: {
        assert(false);
        break;
      }
    }
  }
}

void test2() {
  MPI_Status status;

  if (rank == RANK_MANAGER_NODE) {
    int msg=0;
    // Log node:
    MPI_Send(&msg, 1, MPI_INT32, 
             RANK_LOG_NODE, MPI_TAG_SET_LOG_NODE, MPI_COMM_WORLD);
    MPI_Send(&msg, 1, MPI_INT32, 
             RANK_LOG_NODE, MPI_TAG_LOG_NODE_SET_OUTPUT_COUT, MPI_COMM_WORLD);
    MPI_Recv(&msg, 1, MPI_INT32, 
             RANK_LOG_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD, &status);
    
    // Input_node node:
    MPI_Send(&msg, 1, MPI_INT32, 
             input_node, MPI_TAG_SET_INPUT_NODE, MPI_COMM_WORLD);
    MPI_Recv(&msg, 1, MPI_INT32, 
             input_node, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD, &status);
        
    // Output_node node:
    MPI_Send(&msg, 1, MPI_INT32, 
             output_node, MPI_TAG_SET_OUTPUT_NODE, MPI_COMM_WORLD);
    MPI_Recv(&msg, 1, MPI_INT32, 
             output_node, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD, &status);

    // Connect the input and output:
    // strlen+1 so that \0 gets transmitted as well
    MPI_Send(infile, strlen(infile)+1, MPI_CHAR, 
             input_node, MPI_TAG_SET_DATA_READER_FILE, MPI_COMM_WORLD);
    MPI_Send(outfile, strlen(outfile)+1, MPI_CHAR, 
             output_node, MPI_TAG_SET_DATA_WRITER_FILE, MPI_COMM_WORLD);
    // Set up communication between the input and output node:
    INT32 ranks[2] = {input_node, output_node};
    MPI_Send(ranks, 2, MPI_INT32, 
             input_node, 
             MPI_TAG_ADD_OUTPUT_CONNECTION_MULTIPLE_INPUT_TCP, MPI_COMM_WORLD);

    // Wait until the connection is established
    INT64 channel;
    MPI_Recv(&channel, 1, MPI_INT64, output_node,
             MPI_TAG_INPUT_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &status);

    // set priorities:
    INT64 priority_in[] = {output_node,0,0};
    MPI_Send(&priority_in, 2, MPI_INT64, 
             input_node, MPI_TAG_INPUT_STREAM_SET_PRIORITY, MPI_COMM_WORLD);
             
    INT64 priority_out[] = {input_node, 0};
    MPI_Send(&priority_out, 2, MPI_INT64, 
             output_node, MPI_TAG_OUTPUT_STREAM_SET_PRIORITY, MPI_COMM_WORLD);

    // Wait for data nodes to finish
    MPI_Status status;
    int i;
    MPI_Recv(&i, 1, MPI_INT32, MPI_ANY_SOURCE,
             MPI_TAG_DATASTREAM_EMPTY, MPI_COMM_WORLD, &status);
    assert(status.MPI_TAG == MPI_TAG_DATASTREAM_EMPTY);
    assert(status.MPI_SOURCE == input_node);
   
   
    // Terminate output node
    i=0;
    MPI_Send(&i, 1, MPI_INT32, 
             output_node, MPI_TAG_CORRELATION_READY, MPI_COMM_WORLD);

    MPI_Send(&rank, 1, MPI_INT, 
             RANK_LOG_NODE, MPI_TAG_LOG_MESSAGES_ENDED, MPI_COMM_WORLD);

  } else {
    INT32 msg;
    MPI_Recv(&msg, 1, MPI_INT32, 
             RANK_MANAGER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    
    switch (status.MPI_TAG) {
      case MPI_TAG_SET_LOG_NODE: {
        assert (RANK_LOG_NODE == rank);
        Log_node log_node(rank,numtasks);
        log_node.start();
        break;
      }
      case MPI_TAG_SET_INPUT_NODE: {
        assert(rank == input_node);
        Input_node node(rank);
        node.start();
        break;
      }
      case MPI_TAG_SET_OUTPUT_NODE: {
        assert(rank == output_node);
        Output_node node(rank);
        node.start();
        break;
      }
      default: {
        assert(false);
        break;
      }
    }
  }
}  

int main(int argc, char *argv[]) {
  assert(input_node+output_node+RANK_MANAGER_NODE+RANK_LOG_NODE == 6);



  //initialisation
  int stat = MPI_Init(&argc,&argv);
  if (stat != MPI_SUCCESS) {
    std::cout << "Error starting MPI program. Terminating.\n";
    MPI_Abort(MPI_COMM_WORLD, stat);
  }

  // get the number of tasks set at commandline (= number of processors)
  MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
  // get the ID (rank) of the task, fist rank=0, second rank=1 etc.
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  // 0: controller
  // 1: log node
  // 2: input node
  // 3: output node
  assert(numtasks == 4);
  
  ///////////////////////////
  //  The real work
  ///////////////////////////
  assert(argc == 3);
  infile = argv[1];
  outfile = argv[2];

  // First test
  test2();

  /////////////////////
  MPI_Barrier( MPI_COMM_WORLD );
  /////////////////////

  // Second test
  test1();  

  //close the mpi stuff
  MPI_Barrier( MPI_COMM_WORLD );
  MPI_Finalize();

  return 0;
}
