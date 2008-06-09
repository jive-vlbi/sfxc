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
  for (std::vector< Reader2buffer* >::iterator
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
  case MPI_TAG_ADD_DATA_READER_TCP2: {
      get_log_writer()(3) << print_MPI_TAG(status.MPI_TAG) << std::endl;

      int size;
      MPI_Get_elements(&status, MPI_INT64, &size);
      assert(size >= 3); // stream_nr, [ip-addr]+, port
      uint64_t ip_addr[size];
      MPI_Recv(&ip_addr, size, MPI_INT64, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);

      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

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
      assert((size_t)size > sizeof(int32_t)); // rank + filename
      char msg[size];
      MPI_Recv(&msg, size, MPI_CHAR, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      int32_t stream_nr;
      memcpy(&stream_nr, msg, sizeof(int32_t));
      char *filename = msg+sizeof(int32_t);

      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

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
  assert(i < data_readers.size());
  assert(data_readers[i] != NULL);
  assert(data_readers[i]->get_data_reader() != NULL);
  assert(data_readers[i]->get_queue() == Queue_ptr());

  Queue_ptr queue(new Queue());

  // Make sure a pointer to the data reader has not been returned
  assert(!reader_known[i]);

  data_readers[i]->set_queue(queue);
  data_readers[i]->start();

  buffer_readers[i] = boost::shared_ptr<Reader_buffer>(new Reader_buffer(queue));
}

Multiple_data_readers_controller::Queue_ptr
Multiple_data_readers_controller::get_queue(unsigned int i) {
  if (i < buffer_readers.size())
    return buffer_readers[i]->get_queue();
  return Queue_ptr();
}

boost::shared_ptr<Data_reader>
Multiple_data_readers_controller::get_data_reader(int i) {
  assert((size_t)i < data_readers.size());
  assert(data_readers[i] != NULL);

  reader_known[i] = true;

  if (buffer_readers[i] != boost::shared_ptr<Reader_buffer>()) {
    return buffer_readers[i];
  }
  return data_readers[i]->get_data_reader();
}

bool Multiple_data_readers_controller::initialised(unsigned int i) {
  if (i >= data_readers.size()) return false;
  if (data_readers[i] == NULL) return false;
  return (data_readers[i]->get_data_reader() != NULL);
}

size_t Multiple_data_readers_controller::number_of_data_readers() {
  return data_readers.size();
}


void
Multiple_data_readers_controller::add_data_reader
(int i,
 boost::shared_ptr<Data_reader> reader) {
  // This is false after the first call of get_vector_data_readers()

  if (data_readers.size() <= (unsigned int)i) {
    data_readers.resize(i+1, NULL);
    reader_known.resize(i+1, false);
    buffer_readers.resize(i+1, boost::shared_ptr<Reader_buffer>());
  }
  assert((uint32_t)i < data_readers.size());

  if (data_readers[i] == NULL) {
    data_readers[i] = new Reader2buffer();
  }

  data_readers[i]->set_data_reader(reader);

  node.hook_added_data_reader(i);
}
