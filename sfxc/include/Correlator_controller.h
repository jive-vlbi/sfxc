#ifndef CORRELATOR_CONTROLLER_H
#define CORRELATOR_CONTROLLER_H

#include <Controller.h>
#include <Buffer.h>
#include <Data_reader.h>
#include <Data_writer.h>

/**
 * Correlator_controller processes signals from the Controller_node and
 * forwards them to the correlation
 **/ 
class Correlator_controller : public Controller
{
public:
  typedef Correlator_controller Self;
  typedef Buffer_element<char,131072>    input_value_type;
  typedef Buffer_element<char,131072>    output_value_type;

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
  Data_writer *data_writer;
};

#endif // CORRELATOR_CONTROLLER_H
