#ifndef INPUT_CONTROLLER_H_
#define INPUT_CONTROLLER_H_

#include <Controller.h>
#include <Buffer.h>
#include <Data_reader.h>

class Input_controller : public Controller {
public:
  /// TODO: NGHK: Make this type global?
  typedef char      value_type[131072];
  typedef Input_controller  Self;
  
  Input_controller(Buffer<value_type> &buffer);
  ~Input_controller();

  int process_event(MPI_Status &status);

//  void set_buffer(Buffer<value_type> &buff);
  
  void start();
  void stop();
  bool is_running();

private:
  // For starting the input thread
  static void * start_reading(void *);
  void read();

private:
  Buffer<value_type>       &buffer;
  Data_reader              *reader;
  pthread_t                input_thread;
  bool                     running;
};

#endif /*INPUT_CONTROLLER_H_*/
