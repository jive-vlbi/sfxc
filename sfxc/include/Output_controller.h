#ifndef OUTPUT_CONTROLLER_H
#define OUTPUT_CONTROLLER_H

#include <Controller.h>
#include <Buffer.h>
#include <Data_writer.h>
#include <Data_reader.h>
#include <Log_writer.h>

#include <map>
#include <list>

class Output_controller : public Controller {
public:
  typedef Buffer_element<char, 131072>               value_type;
  typedef Output_controller                          Self;
  
  Output_controller(Buffer<value_type> &buffer, Log_writer &log_writer);

  Process_event_status process_event(MPI_Status &status);  

private:
  class Input_stream {
  public:
    Input_stream() : reader(NULL), buffer(NULL), slice_ready(false) {}
    Data_reader *reader;
    Buffer<value_type> *buffer;
    bool slice_ready;
  };

  //void set_buffer(Buffer<value_type> &buff);
  
  void start_output();
  void stop_output();
  void start_input(Input_stream *reader);


private:
  // For starting the output thread
  static void * start_writing(void *);
  void write_output();

  // For starting one of the input threads
  static void * start_reading(void *);
  void read_output();

  void write_buffers();

private:
  // Output stream:
  Buffer<value_type>                  &output_buffer;
  Data_writer                         *output_writer;
  pthread_t                           output_thread;
  
  // Input streams:
  std::vector< Input_stream >         data_readers;
  std::map<int, int >                 input_streams_order;
};

#endif // OUTPUT_CONTROLLER_H
