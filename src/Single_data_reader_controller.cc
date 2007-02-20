/* Author(s): Nico Kruithof, 2007
 * 
 * $Id: Single_data_reader_controller.cc 153 2007-02-05 09:12:43Z kruithof $
 */


#include <Single_data_reader_controller.h>
#include <sfxc_mpi.h>
#include <Data_reader_file.h>
#include <Data_reader_tcp.h>

Single_data_reader_controller::
Single_data_reader_controller(Log_writer &writer) 
  : Controller(writer) {
}

Single_data_reader_controller::Process_event_status
Single_data_reader_controller::process_event(MPI_Status &status) {
  MPI_Status status2;
  switch (status.MPI_TAG) {
  case MPI_TAG_SET_DATA_READER_FILE:
    {
      log_writer.MPI(2, print_MPI_TAG(status.MPI_TAG));
      int size;
      MPI_Get_elements(&status, MPI_CHAR, &size);
      assert(size > 0);
      char filename[size];
      MPI_Recv(&filename, size, MPI_CHAR, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      if (reader2buffer.get_data_reader() != NULL) {
        reader2buffer.stop();
        reader2buffer.set_data_reader(NULL);
        delete reader2buffer.get_data_reader();
      }
      reader2buffer.set_data_reader(new Data_reader_file(filename));
      assert(reader2buffer.get_buffer() != NULL);
      reader2buffer.start();

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_SET_DATA_READER_TCP:
    {
      log_writer.MPI(2, print_MPI_TAG(status.MPI_TAG));

      int size;
      MPI_Get_elements(&status, MPI_UINT64, &size);
      assert(size > 0);
      UINT64 ip_addr[size];
      MPI_Recv(&ip_addr, size, MPI_UINT64, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      if (reader2buffer.get_data_reader() != NULL) {
        reader2buffer.stop();
        reader2buffer.set_data_reader(NULL);
        delete reader2buffer.get_data_reader();
      }
      Data_reader *data_reader = 
        new Data_reader_tcp(ip_addr, size-1, ip_addr[size-1]);
      reader2buffer.set_data_reader(data_reader);
      assert(reader2buffer.get_buffer() != NULL);
      reader2buffer.start();

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
//  case MPI_TAG_ADD_CORRELATOR_NODE: 
//    {
//      log_writer.MPI(2,"MPI_TAG_ADD_CORRELATOR_NODE");
//      
//      MPI_Status status2;
//      int corr_node;
//      MPI_Recv(&corr_node, 1, MPI_INT32, status.MPI_SOURCE,
//               status.MPI_TAG, MPI_COMM_WORLD, &status2);
//
//      Data_writer_tcp *data_writer = new Data_writer_tcp(1233);
//      node.set_data_writer(corr_node, data_writer);
//
//      TCP_Connection tcp_connection;
//      std::vector<UINT64>  ip_addresses;
//      tcp_connection.get_ip_addresses(ip_addresses);
//
//      //start_output(&data_readers[node]);
//
//      // Add port
//      ip_addresses.push_back(data_writer->get_port());
//      // Add rank
//      //ip_addresses.push_back(node.get_rank()-2);
//      
//      MPI_Send(&ip_addresses[0], ip_addresses.size(), MPI_UINT64, 
//               corr_node, MPI_TAG_SET_DATA_READER_STREAM_TCP, MPI_COMM_WORLD);
//      
//      data_writer->open_connection();
//      
//      node.set_status();      
//      
//      return PROCESS_EVENT_STATUS_SUCCEEDED;
//    }
  }
  return PROCESS_EVENT_STATUS_UNKNOWN;
}

bool 
Single_data_reader_controller::eof() {
  assert(reader2buffer.get_buffer() != NULL);
  if (reader2buffer.get_data_reader() == NULL) {
    return reader2buffer.get_buffer()->empty();
  }
  return (reader2buffer.get_data_reader()->eof() && 
          reader2buffer.get_buffer()->empty());
}

Single_data_reader_controller::Buffer *
Single_data_reader_controller::buffer() {
  return reader2buffer.get_buffer();
}

void 
Single_data_reader_controller::set_buffer(Buffer *buff) {
  assert(buffer() == NULL);
  return reader2buffer.set_buffer(buff);
}
