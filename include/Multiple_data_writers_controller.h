/* Author(s): Nico Kruithof, 2007
 * 
 * $Id: Multiple_data_writer_controller.cc 153 2007-02-05 09:12:43Z kruithof $
 */

#ifndef MULTIPLE_DATA_WRITERS_CONTROLLER_H
#define MULTIPLE_DATA_WRITERS_CONTROLLER_H

#include <Controller.h>

#include <Data_writer.h>
#include <Semaphore_buffer.h>
#include <Buffer2data_writer.h>



class Multiple_data_writers_controller : public Controller {
  typedef Multiple_data_writers_controller  Self;
public:
  typedef Buffer_element<char,131072>      value_type;
  typedef Buffer<value_type>               Buffer;
  typedef Buffer2data_writer<value_type>   Buffer2writer;
  
  Multiple_data_writers_controller(Log_writer &writer);
  ~Multiple_data_writers_controller();
  
  Process_event_status process_event(MPI_Status &status);
  
  Buffer *buffer(unsigned int i);
  void set_buffer(unsigned int i, Buffer *buff);
  
  bool ready();
  
private:
  Buffer2writer &get_writer(unsigned int i);
  Buffer2writer &set_writer(unsigned int i, Data_writer *writer);
  Buffer2writer &create_writer(unsigned int i);

  std::vector< Buffer2data_writer<value_type> >  data_writers;
};

#endif /* MULTIPLE_DATA_WRITERS_CONTROLLER_H */
