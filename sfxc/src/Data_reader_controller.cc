/* Author(s): Nico Kruithof, 2007
 * 
 * $Id$
 */

#include <Input_controller.h>
#include <Data_reader_file.h>
#include <Data_writer_tcp.h>
#include <TCP_Connection.h>
#include <iostream>
#include <assert.h>
#include <signal.h>

Input_controller::Input_controller(Input_node &node) 
  : Controller(node.get_log_writer()), node(node) {
}

Input_controller::Process_event_status
Input_controller::process_event(MPI_Status &status) {
  switch (status.MPI_TAG) {
  case MPI_TAG_SET_INPUT_NODE_FILE:
    {
      log_writer.MPI(2,"MPI_TAG_SET_INPUT_NODE_FILE");
      MPI_Status status2;
      int size;
      MPI_Get_elements(&status, MPI_CHAR, &size);
      assert(size > 0);
      char filename[size];
      MPI_Recv(&filename, size, MPI_CHAR, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_ADD_CORRELATOR_NODE: 
    {
      log_writer.MPI(2,"MPI_TAG_ADD_CORRELATOR_NODE");
      
      MPI_Status status2;
      int corr_node;
      MPI_Recv(&corr_node, 1, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);

      Data_writer_tcp *data_writer = new Data_writer_tcp(1233);
      node.set_data_writer(corr_node, data_writer);

      TCP_Connection tcp_connection;
      std::vector<UINT64>  ip_addresses;
      tcp_connection.get_ip_addresses(ip_addresses);

      //start_output(&data_readers[node]);

      // Add port
      ip_addresses.push_back(data_writer->get_port());
      // Add rank
      //ip_addresses.push_back(node.get_rank()-2);
      
      MPI_Send(&ip_addresses[0], ip_addresses.size(), MPI_UINT64, 
               corr_node, MPI_TAG_SET_INPUT_STREAM_TCP, MPI_COMM_WORLD);
      
      data_writer->open_connection();
      
      node.set_status();      
      
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }
  return PROCESS_EVENT_STATUS_UNKNOWN;
}

