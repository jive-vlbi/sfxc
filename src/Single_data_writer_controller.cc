/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <Single_data_writer_controller.h>
#include <sfxc_mpi.h>
#include <Data_writer_file.h>
#include <Data_writer_tcp.h>
#include <TCP_Connection.h>

Single_data_writer_controller::
Single_data_writer_controller(Node &node) 
  : Controller(node) {
}

Single_data_writer_controller::
~Single_data_writer_controller() {
  buffer2writer.stop();
}

Single_data_writer_controller::Process_event_status
Single_data_writer_controller::process_event(MPI_Status &status) {
  MPI_Status status2;
  switch (status.MPI_TAG) {
  case MPI_TAG_SET_DATA_WRITER_FILE:
    {
      get_log_writer().MPI(0, print_MPI_TAG(status.MPI_TAG));
      int size;
      MPI_Get_elements(&status, MPI_CHAR, &size);
      assert(size > 0);
      char filename[size];
      MPI_Recv(&filename, size, MPI_CHAR, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      boost::shared_ptr<Data_writer> writer(new Data_writer_file(filename));
      set_data_writer(writer);

      INT64 return_msg = 0;
      MPI_Send(&return_msg, 1, MPI_INT64, 
               status.MPI_SOURCE, MPI_TAG_INPUT_CONNECTION_ESTABLISHED, 
               MPI_COMM_WORLD);

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_SET_OUTPUT_CONNECTION_MULTIPLE_INPUT_TCP: 
    {
      get_log_writer().MPI(2, print_MPI_TAG(status.MPI_TAG));
      
      MPI_Status status2;
      INT32 ranks[2]; // Rank of reader and writer:
      MPI_Recv(ranks, 2, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);

      Data_writer_tcp *data_writer = new Data_writer_tcp(1233); 

      TCP_Connection tcp_connection;
      std::vector<UINT64>  ip_addresses;
      tcp_connection.get_ip_addresses(ip_addresses);

      // Add port
      ip_addresses.push_back(data_writer->get_port());
      // Add rank
      ip_addresses.push_back(ranks[0]);
      
      MPI_Send(&ip_addresses[0], ip_addresses.size(), MPI_UINT64, 
               ranks[1], MPI_TAG_ADD_DATA_READER_TCP, MPI_COMM_WORLD);

      data_writer->open_connection();

      boost::shared_ptr<Data_writer> writer(data_writer);
      set_data_writer(writer);

      INT64 return_msg = 0;
      MPI_Recv(&return_msg, 1, MPI_INT64, ranks[1],
               MPI_TAG_INPUT_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &status2);
      MPI_Send(&return_msg, 1, MPI_INT64, 
               status.MPI_SOURCE, MPI_TAG_INPUT_CONNECTION_ESTABLISHED, 
               MPI_COMM_WORLD);

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }
  return PROCESS_EVENT_STATUS_UNKNOWN;
}


boost::shared_ptr<Single_data_writer_controller::Buffer>
Single_data_writer_controller::buffer() {
  return buffer2writer.get_buffer();
}

void Single_data_writer_controller::set_buffer
  (boost::shared_ptr<Single_data_writer_controller::Buffer> buffer) {
  buffer2writer.set_buffer(buffer);
  buffer2writer.try_start();
}

boost::shared_ptr<Data_writer> 
Single_data_writer_controller::get_data_writer(int i) {
  return buffer2writer.get_data_writer();
}


void Single_data_writer_controller::set_data_writer
  (boost::shared_ptr<Data_writer> writer) 
{
  assert(buffer2writer.get_data_writer() == NULL);
  buffer2writer.set_data_writer(writer);
  buffer2writer.try_start();
  
  node.hook_added_data_writer(0);
}
