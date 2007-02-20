// This include ensures that we read the Output_controller class before the
// Output_node class
#include <Output_node.h>

#ifndef OUTPUT_CONTROLLER_H
#define OUTPUT_CONTROLLER_H

#include <Controller.h>
#include <Buffer.h>
#include <Data_writer.h>
#include <Data_reader.h>
#include <Log_writer.h>

#include <map>
#include <list>

// Define the Output_node
class Output_node;


class Output_controller : public Controller {
public:
  typedef Buffer_element<char, 131072>               value_type;
  typedef Output_controller                          Self;
  
  Output_controller(Output_node &node);

  Process_event_status process_event(MPI_Status &status);  

private:

  //void set_buffer(Buffer<value_type> &buff);
  
//  void start_output();
//  void stop_output();
//  void start_input(Input_stream *reader);


private:
//  // For starting the output thread
//  static void * start_writing(void *);
//  void write_output();

//  // For starting one of the input threads
//  static void * start_reading(void *);
//  void read_output();
//
//  void write_buffers();

//  void set_data_reader(int num, Input_stream &stream);

private:
  Output_node &node;
//  // Output stream:
//  Buffer<value_type>                  &output_buffer;
//  Data_writer                         *output_writer;
//  pthread_t                           output_thread;
//  
//  // Input streams:
//  std::vector< Input_stream >         data_readers;
//  std::map<int, int >                 input_streams_order;
};

#endif // OUTPUT_CONTROLLER_H
