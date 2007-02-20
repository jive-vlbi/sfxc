/* Author(s): Nico Kruithof, 2007
 * 
 * $Id: Multiple_data_readers_controller.cc 153 2007-02-05 09:12:43Z kruithof $
 */


#include <Multiple_data_readers_controller.h>
#include <Data_reader_file.h>
#include <Data_reader_tcp.h>
#include <TCP_Connection.h>

Multiple_data_readers_controller::
Multiple_data_readers_controller(Log_writer &writer) 
  : Controller(writer) {
}

Multiple_data_readers_controller::Process_event_status
Multiple_data_readers_controller::process_event(MPI_Status &status) {
  MPI_Status status2;
  switch (status.MPI_TAG) {
  case MPI_TAG_ADD_DATA_READER_TCP:
    {
      log_writer.MPI(2, print_MPI_TAG(status.MPI_TAG));

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
      
      Reader2buffer &input_stream = create_input_stream(corr_node);

      if (input_stream.get_data_reader() != NULL) {
        input_stream.stop();
        delete input_stream.get_data_reader();
        input_stream.set_data_reader(NULL);
      }

      Data_reader_tcp *data_reader = 
        new Data_reader_tcp(ip_addr, size-2, port);
      input_stream.set_data_reader(data_reader);
      input_stream.try_start();

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }
  return PROCESS_EVENT_STATUS_UNKNOWN;
}

Multiple_data_readers_controller::Reader2buffer &
Multiple_data_readers_controller::create_input_stream(unsigned int str) {
  if (data_readers.size() <= str) {
    data_readers.resize(str+1);
  }
  assert(str < data_readers.size());
  return data_readers[str];
}


Multiple_data_readers_controller::Reader2buffer &
Multiple_data_readers_controller::get_input_stream(unsigned int i) {
  assert((unsigned int)i < data_readers.size());
  return data_readers[i];
}

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
