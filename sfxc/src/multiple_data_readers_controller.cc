/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#include "multiple_data_readers_controller.h"
#include "data_reader_file.h"
#include "data_reader_tcp.h"
#include "data_reader_buffer.h"
#include "tcp_connection.h"

Multiple_data_readers_controller::
Multiple_data_readers_controller(Node &node)
    : Controller(node) {}

Multiple_data_readers_controller::
~Multiple_data_readers_controller() {
}

Multiple_data_readers_controller::Process_event_status
Multiple_data_readers_controller::process_event(MPI_Status &status) {
  MPI_Status status2;
  switch (status.MPI_TAG) {
  case MPI_TAG_ADD_DATA_READER_TCP2: {
      get_log_writer()(3) << print_MPI_TAG(status.MPI_TAG) << std::endl;

      int size;
      MPI_Get_elements(&status, MPI_INT64, &size);
      SFXC_ASSERT(size >= 3); // stream_nr, [ip-addr]+, port
      uint64_t ip_addr[size];
      MPI_Recv(&ip_addr, size, MPI_INT64, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);

      SFXC_ASSERT(status.MPI_SOURCE == status2.MPI_SOURCE);
      SFXC_ASSERT(status.MPI_TAG == status2.MPI_TAG);

      int32_t stream_nr = ip_addr[0];
      uint64_t port = ip_addr[size-1];

      boost::shared_ptr<Data_reader>
      reader(new Data_reader_tcp(ip_addr+1, size-2, port));
      add_data_reader(stream_nr, reader);

      MPI_Send(&stream_nr, 1, MPI_INT32,
               status.MPI_SOURCE, MPI_TAG_CONNECTION_ESTABLISHED,
               MPI_COMM_WORLD);

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_ADD_DATA_READER: {
      get_log_writer()(3) << print_MPI_TAG(status.MPI_TAG) << std::endl;

      int size;
      MPI_Get_elements(&status, MPI_CHAR, &size);
      SFXC_ASSERT((size_t)size > sizeof(int32_t)); // rank + filename
      char msg[size];
      MPI_Recv(&msg, size, MPI_CHAR, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      int32_t stream_nr;
      memcpy(&stream_nr, msg, sizeof(int32_t));
      char *filename = msg+sizeof(int32_t);

      SFXC_ASSERT(status.MPI_SOURCE == status2.MPI_SOURCE);
      SFXC_ASSERT(status.MPI_TAG == status2.MPI_TAG);

      boost::shared_ptr<Data_reader> reader(new Data_reader_file(filename));
      add_data_reader(stream_nr, reader);

      MPI_Send(&stream_nr, 1, MPI_INT32,
               status.MPI_SOURCE, MPI_TAG_CONNECTION_ESTABLISHED,
               MPI_COMM_WORLD);

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }
  return PROCESS_EVENT_STATUS_UNKNOWN;
}

void
Multiple_data_readers_controller::
enable_buffering(unsigned int i) {
  SFXC_ASSERT(i < readers.size());
  SFXC_ASSERT(readers[i].reader2buffer != Reader2buffer_ptr());
  SFXC_ASSERT(readers[i].reader2buffer->get_data_reader() != 
              Data_reader_ptr());
  SFXC_ASSERT(readers[i].reader2buffer->get_queue() == Queue_ptr());

  Queue_ptr queue(new Queue());

  readers[i].reader2buffer->set_queue(queue);
  readers[i].reader2buffer->start();

  readers[i].reader_buffer = Reader_buffer_ptr(new Reader_buffer(queue));
}

Multiple_data_readers_controller::Queue_ptr
Multiple_data_readers_controller::get_queue(unsigned int i) {
  if (i < readers.size())
    return readers[i].reader2buffer->get_queue();
  return Queue_ptr();
}

Multiple_data_readers_controller::Data_reader_ptr
Multiple_data_readers_controller::get_data_reader(int i) {
  SFXC_ASSERT((size_t)i < readers.size());
  SFXC_ASSERT(readers[i].reader2buffer != Reader2buffer_ptr());

  if (readers[i].reader_buffer != Reader_buffer_ptr())
    return readers[i].reader_buffer;
  
  return readers[i].reader2buffer->get_data_reader();
}

bool Multiple_data_readers_controller::initialised(unsigned int i) {
  if (i >= readers.size()) return false;
  if (readers[i].reader2buffer == Reader2buffer_ptr()) return false;
  return (readers[i].reader2buffer->get_data_reader() != 
          Data_reader_ptr());
}

size_t Multiple_data_readers_controller::number_of_data_readers() {
  return readers.size();
}


void
Multiple_data_readers_controller::add_data_reader
(int i,
 boost::shared_ptr<Data_reader> reader) {
  // This is false after the first call of get_vector_data_readers()

  if (readers.size() <= (unsigned int)i) {
    readers.resize(i+1);
  }
  SFXC_ASSERT((uint32_t)i < readers.size());

  if (readers[i].reader2buffer == Reader2buffer_ptr()) {
    readers[i].reader2buffer = Reader2buffer_ptr(new Reader2buffer());
  }

  readers[i].reader2buffer->set_data_reader(reader);

  node.hook_added_data_reader(i);
}
