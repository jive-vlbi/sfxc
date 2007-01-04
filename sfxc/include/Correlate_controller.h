#ifndef CORRELATE_CONTROLLER_H
#define CORRELATE_CONTROLLER_H

#include <Controller.h>
#include <Buffer.h>
#include <Data_reader.h>

/**
 * Correlate_controller processes signals from the Controller_node and
 * forwards them to the correlation
 **/ 
class Correlate_controller : public Controller
{
public:
  typedef Correlate_controller Self;
  typedef char                 input_value_type[131072];
  typedef char                 output_value_type[131072];

  Correlate_controller(Buffer<output_value_type> &output_buffer);
  ~Correlate_controller();
  
  int process_event(MPI_Status &status);
  
  void start();
  void stop();
  bool is_running();
  
private:

  static void *start_correlating(void *self_);
  void correlate();

  Buffer<output_value_type>      &output_buffer;
  bool running;
  pthread_t                      correlate_thread;
  std::vector<Data_reader *>     data_readers;
  int curr_station;
};

#endif // CORRELATE_CONTROLLER_H
