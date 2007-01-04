#include <Input_controller.h>
#include <Data_reader_file.h>
#include <iostream>
#include <assert.h>
#include <signal.h>

Input_controller::Input_controller(Buffer<value_type> &buffer) 
  : Controller(), buffer(buffer), reader(NULL), running(false) {
}

Input_controller::~Input_controller() {
  stop();
  //pthread_kill(input_thread, SIGKILL);
}

int
Input_controller::process_event(MPI_Status &status) {
  switch (status.MPI_TAG) {
  case MPI_TAG_SET_INPUT_NODE_FILE:
    {
      MPI_Status status2;
      int size;
      MPI_Get_elements(&status, MPI_CHAR, &size);
      assert(size > 0);
      char filename[size];
      MPI_Recv(&filename, size, MPI_CHAR, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      if (reader != NULL) delete reader;
      reader = new Data_reader_file(filename);

      if (reader != NULL) {
        start();
      }
      
      return 0;
    }
  }
  return 1;
}

void
Input_controller::start() {
  pthread_create(&input_thread, NULL, start_reading, static_cast<void*>(this));
  running = true;
}
void
Input_controller::stop() {
  running = false;
}
bool
Input_controller::is_running() {
  return running;
}
void *
Input_controller::start_reading(void *self_) {
  Self *self = static_cast<Self *>(self_);
  self->read();
  return NULL;
}

void
Input_controller::read() {
  while (running) {
    value_type &t = buffer.produce();
    UINT64 size = reader->get_bytes(sizeof(value_type), t);
    buffer.produced(size);
    if (size == 0) {
      running = false;
    }
  }
}
