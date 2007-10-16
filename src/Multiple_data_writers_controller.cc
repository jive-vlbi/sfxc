/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <Multiple_data_writers_controller.h>
#include <sfxc_mpi.h>
#include <Data_writer_file.h>
#include <Data_writer_tcp.h>
#include <Data_writer_void.h>
#include <TCP_Connection.h>

#include <Semaphore_buffer.h>

Multiple_data_writers_controller::
Multiple_data_writers_controller(Node &node) 
  : Controller(node) {
}

Multiple_data_writers_controller::
~Multiple_data_writers_controller() {
  for (std::vector< Output_stream >::iterator 
         it = data_writers.begin(); it != data_writers.end(); it++) {
    if ((*it).buffer2writer != NULL) {
      delete it->buffer2writer;
    }
  }
}


boost::shared_ptr<Data_writer>
Multiple_data_writers_controller::get_data_writer(size_t i) {
  assert(i < data_writers.size());
  return data_writers[i].buffer2writer->get_data_writer();
}

Multiple_data_writers_controller::Process_event_status
Multiple_data_writers_controller::process_event(MPI_Status &status) {
  switch (status.MPI_TAG) {
  case MPI_TAG_ADD_TCP: 
    {
      get_log_writer()(2) << print_MPI_TAG(status.MPI_TAG) << std::endl;
      
      MPI_Status status2;

      /* - int32_t: stream number for the data writer
       * - int32_t: stream number for the data reader
       * - int32_t: rank of the data_reader
       */
      int32_t ranks[3]; 
      MPI_Recv(ranks, 3, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);

      Data_writer_tcp *data_writer = new Data_writer_tcp(1233); 

      TCP_Connection tcp_connection;
      std::vector<uint64_t>  ip_addresses;
      // Add number of the data stream:
      ip_addresses.push_back(ranks[2]);
      // Add the ip addresses
      tcp_connection.get_ip_addresses(ip_addresses);

      // Add port
      ip_addresses.push_back(data_writer->get_port());
      
      MPI_Send(&ip_addresses[0], ip_addresses.size(), MPI_INT64, 
               ranks[1], MPI_TAG_ADD_DATA_READER_TCP2, MPI_COMM_WORLD);

      data_writer->open_connection();

      boost::shared_ptr<Data_writer> writer(data_writer);
      add_data_writer(ranks[0], writer, ranks[2], ranks[1]);

      int32_t return_msg = 0;
      MPI_Recv(&return_msg, 1, MPI_INT32, ranks[1],
               MPI_TAG_CONNECTION_ESTABLISHED, MPI_COMM_WORLD, &status2);

      MPI_Send(&ranks[0], 1, MPI_INT32, 
               status.MPI_SOURCE, MPI_TAG_CONNECTION_ESTABLISHED, 
               MPI_COMM_WORLD);

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_ADD_DATA_WRITER_FILE2: 
    {
      get_log_writer()(2) << print_MPI_TAG(status.MPI_TAG) << std::endl;
      
      MPI_Status status2;
      int size;
      MPI_Get_elements(&status, MPI_CHAR, &size);
      assert(size > 0);
      char msg[size];
      MPI_Recv(&msg, size, MPI_CHAR, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      int stream_nr;
      memcpy(&stream_nr, msg, sizeof(int32_t));
      char *filename = msg+sizeof(int32_t);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      boost::shared_ptr<Data_writer> 
        data_writer(new Data_writer_file(filename)); 

      add_data_writer(stream_nr, data_writer, -1, -1);

      MPI_Send(&stream_nr, 1, MPI_INT32, 
               status.MPI_SOURCE, MPI_TAG_CONNECTION_ESTABLISHED, 
               MPI_COMM_WORLD);

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_ADD_DATA_WRITER_VOID2: 
    {
      get_log_writer()(2) << print_MPI_TAG(status.MPI_TAG) << std::endl;
      
      MPI_Status status2;
      int32_t msg;
      MPI_Recv(&msg, 1, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      boost::shared_ptr<Data_writer> data_writer(new Data_writer_void()); 

      add_data_writer(msg, data_writer, -1, -1);

      MPI_Send(&msg, 1, MPI_INT32, 
               status.MPI_SOURCE, MPI_TAG_CONNECTION_ESTABLISHED, 
               MPI_COMM_WORLD);

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }
  return PROCESS_EVENT_STATUS_UNKNOWN;
}


Multiple_data_writers_controller::Buffer2writer &
Multiple_data_writers_controller::get_buffer2writer(unsigned int i) {
  assert((unsigned int)i < data_writers.size());
  if (data_writers[i].buffer2writer == NULL) {
    std::cout << "data_writers[i].buffer2writer == NULL: " << node.get_rank() << std::endl;
  }
  assert(data_writers[i].buffer2writer != NULL);
  return *data_writers[i].buffer2writer;
}

boost::shared_ptr<Multiple_data_writers_controller::Buffer>
Multiple_data_writers_controller::buffer(unsigned int i) {
  return get_buffer2writer(i).get_buffer();
}

void 
Multiple_data_writers_controller::set_buffer
  (unsigned int i, 
   boost::shared_ptr<Multiple_data_writers_controller::Buffer> buff) 
{
  get_buffer2writer(i).set_buffer(buff);
  get_buffer2writer(i).try_start();
  
}


Multiple_data_writers_controller::Buffer2writer *
Multiple_data_writers_controller::operator[](int i) {
  assert((size_t)i < data_writers.size());
  
  return data_writers[i].buffer2writer;
}

bool 
Multiple_data_writers_controller::ready() {
  for (size_t i=0; i<data_writers.size(); i++) {
    if (data_writers[i].buffer2writer != NULL) {
      if (data_writers[i].buffer2writer->get_buffer() != NULL) {
        assert(data_writers[i].buffer2writer->get_data_writer() != NULL);
        if (!data_writers[i].buffer2writer->get_buffer()->empty()) {
          return false;
        }
      }
    }
  }
  return true;
}

void 
Multiple_data_writers_controller::
add_data_writer(unsigned int i, boost::shared_ptr<Data_writer> writer, 
                int rank_node_reader_, int stream_number_reader_) {
  if (data_writers.size() <= i) {
    data_writers.resize(i+1);
  }
  assert(i < data_writers.size());
  
  data_writers[i].rank_node_reader = rank_node_reader_;
  data_writers[i].stream_number_reader = stream_number_reader_;
  if (data_writers[i].buffer2writer == NULL) {
    data_writers[i].buffer2writer = new Buffer2data_writer<value_type>();
  }
  
  data_writers[i].buffer2writer->set_data_writer(writer);
  
  node.hook_added_data_writer(i);
}

