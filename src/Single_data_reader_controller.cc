/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <Single_data_reader_controller.h>
#include <sfxc_mpi.h>
#include <Data_reader_file.h>
#include <Data_reader_tcp.h>

Single_data_reader_controller::
Single_data_reader_controller(Node &node) 
  : Controller(node) {
}

Single_data_reader_controller::Process_event_status
Single_data_reader_controller::process_event(MPI_Status &status) {
  MPI_Status status2;
  switch (status.MPI_TAG) {
  case MPI_TAG_SET_DATA_READER_FILE:
    {
      get_log_writer().MPI(2, print_MPI_TAG(status.MPI_TAG));
      int size;
      MPI_Get_elements(&status, MPI_CHAR, &size);
      assert(size > 0);
      char filename[size];
      MPI_Recv(&filename, size, MPI_CHAR, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      set_data_reader(new Data_reader_file(filename));

      INT64 msg = 0;
      MPI_Send(&msg, 1, MPI_INT64, 
               RANK_MANAGER_NODE, MPI_TAG_INPUT_CONNECTION_ESTABLISHED, 
               MPI_COMM_WORLD);

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_SET_DATA_READER_TCP:
    {
      int size;
      MPI_Get_elements(&status, MPI_UINT64, &size);
      assert(size > 0);
      UINT64 ip_addr[size];
      MPI_Recv(&ip_addr, size, MPI_UINT64, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      set_data_reader(new Data_reader_tcp(ip_addr, size-1, ip_addr[size-1]));

      INT64 msg = status.MPI_SOURCE;
      MPI_Send(&msg, 1, MPI_INT64, 
               RANK_MANAGER_NODE, MPI_TAG_INPUT_CONNECTION_ESTABLISHED, 
               MPI_COMM_WORLD);

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }
  return PROCESS_EVENT_STATUS_UNKNOWN;
}

bool 
Single_data_reader_controller::eof() {
  if (reader2buffer.get_data_reader() != NULL) {
    if (!reader2buffer.get_data_reader()->eof()) return false;
  }
    
  if (reader2buffer.get_buffer() != NULL) {
    return (reader2buffer.get_buffer()->empty());
  }
  return true;
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

Data_reader *
Single_data_reader_controller::get_data_reader() {
  assert(reader2buffer.get_data_reader() != NULL);
  return reader2buffer.get_data_reader();
}

void 
Single_data_reader_controller::set_data_reader(Data_reader *reader) {
  if (reader2buffer.get_data_reader() != NULL) {
    reader2buffer.stop();
    delete reader2buffer.get_data_reader();
    reader2buffer.set_data_reader(NULL);
  }

  reader2buffer.set_data_reader(reader);
  reader2buffer.try_start();

  node.hook_added_data_reader(0);
}
