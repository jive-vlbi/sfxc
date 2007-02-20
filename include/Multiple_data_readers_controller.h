/* Author(s): Nico Kruithof, 2007
 * 
 * $Id: Multiple_data_writer_controller.cc 153 2007-02-05 09:12:43Z kruithof $
 */

#ifndef MULTIPLE_DATA_READERS_CONTROLLER_H
#define MULTIPLE_DATA_READERS_CONTROLLER_H

#include <Controller.h>

#include <Data_writer.h>
#include <Semaphore_buffer.h>
#include <Data_reader2buffer.h>



class Multiple_data_readers_controller : public Controller {
  typedef Multiple_data_readers_controller  Self;
public:
  /// TODO: NGHK: Make this type global?
  typedef Buffer_element<char,131072>      value_type;
  typedef Data_reader2buffer<value_type>   Reader2buffer;
  typedef Buffer<value_type>               Buffer;
  
  Multiple_data_readers_controller(Log_writer &writer);

  Process_event_status process_event(MPI_Status &status);
  
  Buffer *get_buffer(unsigned int i);
  void set_buffer(unsigned int i, Buffer *buffer);
private:
  Reader2buffer &create_input_stream(unsigned int i);
  Reader2buffer &get_input_stream(unsigned int i);

  std::vector<Data_reader2buffer<value_type> >   data_readers;
};

#endif /* MULTIPLE_DATA_READERS_CONTROLLER_H */
