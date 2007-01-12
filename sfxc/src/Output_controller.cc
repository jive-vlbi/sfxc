#include <Output_controller.h>
#include <iostream>
#include <assert.h>

#include <Data_writer_file.h>

Output_controller::Output_controller(Buffer<value_type> &buffer)
  : Controller(), buffer(buffer), writer(NULL) {
}

Controller::Process_event_status
Output_controller::process_event(MPI_Status &status) {
  switch (status.MPI_TAG) {
  case MPI_TAG_SET_OUTPUT_NODE_FILE:
    {
      MPI_Status status2;
      int size;
      MPI_Get_elements(&status, MPI_CHAR, &size);
      char filename[size];
      MPI_Recv(&filename, size, MPI_CHAR, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      if (writer != NULL) delete writer;
      writer = new Data_writer_file(filename);

      if (writer != NULL) {
        start();
      }
      
      
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }
  return PROCESS_EVENT_STATUS_UNKNOWN;
}

void
Output_controller::start() {
  pthread_create(&output_thread, NULL, 
                 start_writing, static_cast<void*>(this));
  running = true;
}
void
Output_controller::stop() {
  running = false;
}
bool
Output_controller::is_running() {
  return running;
}
void *
Output_controller::start_writing(void *self_) {
  Self *self = static_cast<Self *>(self_);
  self->write();

  // Writing ended, notify controller node
  int i=0; 
  MPI_Send(&i, 1, MPI_INT, 0,
           MPI_MSG_DATASTREAM_EMPTY, MPI_COMM_WORLD);

  return NULL;
}

void
Output_controller::write() {
  while (running) {
    int size;
    value_type &t = buffer.consume(size);
    writer->put_bytes(size, t);
    buffer.consumed();
    running = (size != 0);
  }
}
