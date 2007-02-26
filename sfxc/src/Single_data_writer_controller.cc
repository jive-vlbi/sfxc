/* Author(s): Nico Kruithof, 2007
 * 
 * $Id: Single_data_writer_controller.cc 153 2007-02-05 09:12:43Z kruithof $
 */


#include <Single_data_writer_controller.h>
#include <sfxc_mpi.h>
#include <Data_writer_file.h>
#include <Data_writer_tcp.h>

Single_data_writer_controller::
Single_data_writer_controller(Node &node) 
  : Controller(node) {
}

Single_data_writer_controller::
~Single_data_writer_controller() {
  buffer2writer.stop();
  // Don't delete the buffers. 
  // This should be done by the node that also created them.
  if (buffer2writer.get_data_writer() != NULL) {
    delete buffer2writer.get_data_writer();
    buffer2writer.set_data_writer(NULL);
  }
}

Single_data_writer_controller::Process_event_status
Single_data_writer_controller::process_event(MPI_Status &status) {
  MPI_Status status2;
  switch (status.MPI_TAG) {
  case MPI_TAG_SET_DATA_WRITER_FILE:
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

      set_data_writer(new Data_writer_file(filename));

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }
  return PROCESS_EVENT_STATUS_UNKNOWN;
}


Single_data_writer_controller::Buffer *
Single_data_writer_controller::buffer() {
  return buffer2writer.get_buffer();
}

void Single_data_writer_controller::set_buffer(Buffer *buffer) {
  buffer2writer.set_buffer(buffer);
  buffer2writer.try_start();
}

void Single_data_writer_controller::set_data_writer(Data_writer *writer) {
  if (buffer2writer.get_data_writer() != NULL) {
    buffer2writer.stop();
    buffer2writer.set_data_writer(NULL);
    delete buffer2writer.get_data_writer();
  }
  buffer2writer.set_data_writer(writer);
  buffer2writer.try_start();
  
  node.hook_added_data_writer(0);
}
