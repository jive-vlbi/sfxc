/* Author(s): Nico Kruithof, 2007
 * 
 * $Id: Multiple_data_writers_controller.cc 153 2007-02-05 09:12:43Z kruithof $
 */


#include <Multiple_data_writers_controller.h>
#include <sfxc_mpi.h>
#include <Data_writer_file.h>
#include <Data_writer_tcp.h>
#include <TCP_Connection.h>

#include <Semaphore_buffer.h>

Multiple_data_writers_controller::
Multiple_data_writers_controller(Node &node) 
  : Controller(node) {
}

Multiple_data_writers_controller::
~Multiple_data_writers_controller() {
  for (std::vector< Buffer2data_writer<value_type>* >::iterator 
         it = data_writers.begin(); it != data_writers.end(); it++) {
    if ((*it) != NULL) {
      (*it)->stop();
      // Don't delete the buffers. 
      // This should be done by the node that also created them.
      if ((*it)->get_data_writer() != NULL) {
        delete (*it)->get_data_writer();
        (*it)->set_data_writer(NULL);
      }
    }
  }
}

Multiple_data_writers_controller::Process_event_status
Multiple_data_writers_controller::process_event(MPI_Status &status) {
  switch (status.MPI_TAG) {
  case MPI_TAG_ADD_OUTPUT_CONNECTION_SINGLE_INPUT_TCP: 
    {
      get_log_writer().MPI(2, print_MPI_TAG(status.MPI_TAG));
      
      MPI_Status status2;
      INT32 corr_node;
      MPI_Recv(&corr_node, 1, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);

      Data_writer_tcp *data_writer = new Data_writer_tcp(1233); 

      TCP_Connection tcp_connection;
      std::vector<UINT64>  ip_addresses;
      tcp_connection.get_ip_addresses(ip_addresses);

      // Add port
      ip_addresses.push_back(data_writer->get_port());
      
      MPI_Send(&ip_addresses[0], ip_addresses.size(), MPI_UINT64, 
               corr_node, MPI_TAG_SET_DATA_READER_TCP, MPI_COMM_WORLD);
      
      data_writer->open_connection();

      add_data_writer(corr_node, data_writer);
      
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_ADD_OUTPUT_CONNECTION_MULTIPLE_INPUT_TCP: 
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

      add_data_writer(ranks[1], data_writer);

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_ADD_DATA_WRITER_FILE: 
    {
      get_log_writer().MPI(2, print_MPI_TAG(status.MPI_TAG));
      
      MPI_Status status2;
      int size;
      MPI_Get_elements(&status, MPI_CHAR, &size);
      assert(size > 0);
      char buffer[size], filename[size-4];
      MPI_Recv(&buffer[0], size, MPI_PACKED, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      int position=0;
      INT32 corr_node;
      MPI_Unpack(buffer, size, &position, &corr_node, 1, MPI_INT32, MPI_COMM_WORLD);
      MPI_Unpack(buffer, size, &position, filename, size-4, MPI_CHAR, MPI_COMM_WORLD);
      assert(position == size);

      Data_writer_file *data_writer = new Data_writer_file(filename); 

      add_data_writer(corr_node, data_writer);
      
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }
  return PROCESS_EVENT_STATUS_UNKNOWN;
}


Multiple_data_writers_controller::Buffer2writer &
Multiple_data_writers_controller::get_writer(unsigned int i) {
  assert((unsigned int)i < data_writers.size());
  assert(data_writers[i] != NULL);
  return *data_writers[i];
}

Multiple_data_writers_controller::Buffer *
Multiple_data_writers_controller::buffer(unsigned int i) {
  return get_writer(i).get_buffer();
}

void 
Multiple_data_writers_controller::set_buffer(unsigned int i, Buffer *buff) {
  get_writer(i).set_buffer(buff);
  get_writer(i).try_start();
  
}

bool 
Multiple_data_writers_controller::ready() {
  for (unsigned int i=0; i<data_writers.size(); i++) {
    if (data_writers[i] != NULL) {
      if (data_writers[i]->get_buffer() != NULL) {
        assert(data_writers[i]->get_data_writer() != NULL);
        if (!data_writers[i]->get_buffer()->empty()) {
          return false;
        }
      }
    }
  }
  return true;
}

void 
Multiple_data_writers_controller::
add_data_writer(unsigned int i, Data_writer *writer) {
  if (data_writers.size() <= i) {
    data_writers.resize(i+1, NULL);
  }
  assert(i < data_writers.size());

  if (data_writers[i] == NULL) {
    data_writers[i] = new Buffer2data_writer<value_type>();
  }
  Buffer2writer &buffer2writer = *data_writers[i];
  assert(buffer2writer.get_data_writer() == NULL);
  assert(buffer2writer.get_buffer() == NULL);
  
  buffer2writer.set_data_writer(writer);

  buffer2writer.try_start();
  
  node.hook_added_data_writer(i);
}
