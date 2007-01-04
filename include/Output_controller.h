#ifndef OUTPUT_CONTROLLER_H
#define OUTPUT_CONTROLLER_H

#include <Controller.h>
#include <Buffer.h>
#include <Data_writer.h>

class Output_controller : public Controller {
public:
  typedef char       value_type[131072];
  typedef Output_controller  Self;
  Output_controller(Buffer<value_type> &buffer);

  int process_event(MPI_Status &status);  


  //void set_buffer(Buffer<value_type> &buff);
  
  void start();
  void stop();
  bool is_running();

private:
  /// TODO: move start_writing to the Data_reader (in set_buffer)
  // For starting the input thread
  static void * start_writing(void *);
  void write();

private:
  Buffer<value_type>       &buffer;
  Data_writer              *writer;
  pthread_t                output_thread;
  bool                     running;
};

#endif // OUTPUT_CONTROLLER_H
