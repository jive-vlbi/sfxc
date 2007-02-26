/* Author(s): Nico Kruithof, 2007
 * 
 * $Id: Multiple_data_readers_controller.cc 153 2007-02-05 09:12:43Z kruithof $
 */


#include <Multiple_data_readers_controller.h>
#include <Data_reader_file.h>
#include <Data_reader_tcp.h>
#include <Data_reader_buffer.h>
#include <TCP_Connection.h>

Multiple_data_readers_controller::
Multiple_data_readers_controller(Node &node) 
  : Controller(node), more_data_readers_can_be_added(true) {
}

Multiple_data_readers_controller::
~Multiple_data_readers_controller() {
  for (std::vector<Data_reader2buffer<value_type> >::iterator 
         it = data_readers.begin(); it != data_readers.end(); it++) {
    it->stop();
    // Don't delete the buffers. 
    // This should be done by the node that also created them.
    if ((*it).get_data_reader() != NULL) {
      delete (*it).get_data_reader();
      (*it).set_data_reader(NULL);
    }
  }
}

Multiple_data_readers_controller::Process_event_status
Multiple_data_readers_controller::process_event(MPI_Status &status) {
  MPI_Status status2;
  switch (status.MPI_TAG) {
  case MPI_TAG_ADD_DATA_READER_TCP:
    {
      get_log_writer().MPI(2, print_MPI_TAG(status.MPI_TAG));

      int size;
      MPI_Get_elements(&status, MPI_UINT64, &size);
      assert(size >= 3); // [ip-addr]+, port, rank
      UINT64 ip_addr[size];
      MPI_Recv(&ip_addr, size, MPI_UINT64, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);

      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);
      
      UINT64 port = ip_addr[size-2];
      UINT64 corr_node = ip_addr[size-1];
      
      add_data_reader(corr_node, new Data_reader_tcp(ip_addr, size-2, port));

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_ADD_DATA_READER_FILE:
    {
      get_log_writer().MPI(2, print_MPI_TAG(status.MPI_TAG));

      int size;
      MPI_Get_elements(&status, MPI_CHAR, &size);
      assert(size > 1); // rank + filename
      char ip_addr[size];
      MPI_Recv(&ip_addr, size, MPI_CHAR, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);

      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);
      
      UINT64 corr_node = (int)ip_addr[0];
      char *filename = ip_addr+1;
      
      add_data_reader(corr_node, new Data_reader_file(filename));

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }
  return PROCESS_EVENT_STATUS_UNKNOWN;
}


//Multiple_data_readers_controller::Reader2buffer &
//Multiple_data_readers_controller::get_input_stream(unsigned int i) {
//  assert((unsigned int)i < data_readers.size());
//  return data_readers[i];
//}

Multiple_data_readers_controller::Buffer *
Multiple_data_readers_controller::get_buffer(unsigned int i) {
  if (i >= data_readers.size()) return NULL;
  return data_readers[i].get_buffer();
}

void 
Multiple_data_readers_controller::set_buffer(unsigned int i, Buffer *buffer) {
  if (data_readers.size() <= i) {
    data_readers.resize(i+1);
  }
  assert(i < data_readers.size());
  data_readers[i].set_buffer(buffer);
  data_readers[i].try_start();
}

bool Multiple_data_readers_controller::initialised(unsigned int i) {
  if (i >= data_readers.size()) return false;
  return (data_readers[i].get_data_reader() != NULL);
}

unsigned int Multiple_data_readers_controller::number_of_data_readers() {
  return data_readers.size();
}

std::vector<Data_reader *> &
Multiple_data_readers_controller::get_vector_data_readers() {
  if (data_readers_out.empty()) {
    //input_readers_out.resize(data_readers.size());
    for (unsigned int i=0; i<data_readers.size(); i++) {
      if (data_readers[i].get_buffer() != NULL) {
        // This is an active data_reader
        data_readers_out.push_back(new Data_reader_buffer(data_readers[i].get_buffer()));
      }
    }
    more_data_readers_can_be_added = false;
  }
  return data_readers_out;
}

void 
Multiple_data_readers_controller::add_data_reader(int i, Data_reader *reader) {
  // This is false after the first call of get_vector_data_readers()
  assert(more_data_readers_can_be_added);
  
  if (data_readers.size() <= (unsigned int)i) {
    data_readers.resize(i+1);
  }
  assert((UINT32)i < data_readers.size());

  Reader2buffer &input_stream = data_readers[i];

  if (input_stream.get_data_reader() != NULL) {
    input_stream.stop();
    delete input_stream.get_data_reader();
    input_stream.set_data_reader(NULL);
  }

  input_stream.set_data_reader(reader);
  input_stream.try_start();
  
  node.hook_added_data_reader(i);
}
