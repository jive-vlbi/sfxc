#ifndef CORRELATOR_CONTROLLER_H
#define CORRELATOR_CONTROLLER_H

#include <Controller.h>
#include <Buffer.h>
#include <Data_reader.h>

/**
 * Correlator_controller processes signals from the Controller_node and
 * forwards them to the correlation
 **/ 
class Correlator_controller : public Controller
{
public:
  typedef Correlator_controller Self;
  typedef char                 input_value_type[131072];
  typedef char                 output_value_type[131072];

  Correlator_controller(Buffer<output_value_type> &output_buffer, 
                       Log_writer &log_writer);
  ~Correlator_controller();
  
  Process_event_status process_event(MPI_Status &status);
  
  void start();
  void stop();
  bool is_running();
  
private:

  static void *start_correlating(void *self_);
  void correlate();

  Buffer<output_value_type>      &output_buffer;
  bool running;
  pthread_t                      correlator_thread;
  std::vector<Data_reader *>     data_readers;
  int curr_station;
};

#endif // CORRELATOR_CONTROLLER_H
