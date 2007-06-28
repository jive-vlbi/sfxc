/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <Multiple_data_readers_controller.h>
#include <Data_reader_file.h>
#include <Data_reader_tcp.h>
#include <Data_reader_buffer.h>
#include <TCP_Connection.h>

Multiple_data_readers_controller::
Multiple_data_readers_controller(Node &node) 
  : Controller(node) {
}

Multiple_data_readers_controller::
~Multiple_data_readers_controller() {
  for (std::vector< Data_reader2buffer<value_type>* >::iterator 
         it = data_readers.begin(); it != data_readers.end(); it++) {
    if ((*it) != NULL) {
      delete *it;
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
      INT64 stream_nr = ip_addr[size-1];
      
      boost::shared_ptr<Data_reader> 
        reader(new Data_reader_tcp(ip_addr, size-2, port));
      add_data_reader(stream_nr, reader);

      MPI_Send(&stream_nr, 1, MPI_INT64, 
               status.MPI_SOURCE, MPI_TAG_INPUT_CONNECTION_ESTABLISHED, 
               MPI_COMM_WORLD);
      
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_ADD_DATA_READER_FILE:
    {
      assert(false);
      get_log_writer().MPI(2, print_MPI_TAG(status.MPI_TAG));

      int size;
      MPI_Get_elements(&status, MPI_CHAR, &size);
      assert(size > 1); // rank + filename
      DEBUG_MSG(size);
      char msg[size];
      DEBUG_MSG("");
      MPI_Recv(&msg, size, MPI_CHAR, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      DEBUG_MSG("");

      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);
      
      int corr_node = (int)msg[0];
      char *filename = msg+1;
      DEBUG_MSG(corr_node << " " << filename);
      
      boost::shared_ptr<Data_reader> reader(new Data_reader_file(filename));
      add_data_reader(corr_node, reader);

      DEBUG_MSG("");
      INT64 return_msg = 0;
      MPI_Send(&return_msg, 1, MPI_INT64, 
               status.MPI_SOURCE, MPI_TAG_INPUT_CONNECTION_ESTABLISHED, 
               MPI_COMM_WORLD);
      DEBUG_MSG("");

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }
  return PROCESS_EVENT_STATUS_UNKNOWN;
}

boost::shared_ptr<Multiple_data_readers_controller::Buffer>
Multiple_data_readers_controller::get_buffer(unsigned int i) {
  if (i >= data_readers.size()) 
    return boost::shared_ptr<Multiple_data_readers_controller::Buffer>();
  if (data_readers[i] == NULL) 
    return boost::shared_ptr<Multiple_data_readers_controller::Buffer>();
  return data_readers[i]->get_buffer();
}

void 
Multiple_data_readers_controller::set_buffer(unsigned int i, 
                                             boost::shared_ptr<Buffer> buffer) {
  assert(i < data_readers.size());
  assert(data_readers[i] != NULL);
  assert(data_readers[i]->get_data_reader() != NULL);
  assert(buffer != NULL);
  
  data_readers[i]->set_buffer(buffer);
  data_readers[i]->start();
}

boost::shared_ptr<Data_reader>
Multiple_data_readers_controller::get_data_reader(int i) {
  assert((size_t)i < data_readers.size());
  assert(data_readers[i] != NULL);
  return data_readers[i]->get_data_reader();
}

Multiple_data_readers_controller::Reader2buffer *
Multiple_data_readers_controller::get_data_reader2buffer(int i) {
  assert((size_t)i < data_readers.size());
  return data_readers[i];
}

bool Multiple_data_readers_controller::initialised(unsigned int i) {
  if (i >= data_readers.size()) return false;
  if (data_readers[i] == NULL) return false;
  return (data_readers[i]->get_data_reader() != NULL);
}

unsigned int Multiple_data_readers_controller::number_of_data_readers() {
  return data_readers.size();
}


void 
Multiple_data_readers_controller::add_data_reader
  (int i,
   boost::shared_ptr<Data_reader> reader) 
{
  // This is false after the first call of get_vector_data_readers()
  
  if (data_readers.size() <= (unsigned int)i) {
    data_readers.resize(i+1, NULL);
  }
  assert((UINT32)i < data_readers.size());

  if (data_readers[i] == NULL) {
    data_readers[i] = new Data_reader2buffer<value_type>();
  }
  
  data_readers[i]->set_data_reader(reader);
  
  node.hook_added_data_reader(i);
}
