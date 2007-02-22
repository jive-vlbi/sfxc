/* Author(s): Nico Kruithof, 2007
 * 
 * $Id: Single_data_writer_controller.cc 153 2007-02-05 09:12:43Z kruithof $
 */

#ifndef SINGLE_DATA_WRITER_CONTROLLER_H
#define SINGLE_DATA_WRITER_CONTROLLER_H

#include <Controller.h>

#include <Data_writer.h>
#include <Semaphore_buffer.h>
#include <Buffer2data_writer.h>

class Single_data_writer_controller : public Controller {
  typedef Single_data_writer_controller  Self;
public:
  typedef Buffer_element<char,131072>      value_type;
  typedef Buffer<value_type>               Buffer;
  
  Single_data_writer_controller(Log_writer &writer);
  ~Single_data_writer_controller();

  Process_event_status process_event(MPI_Status &status);
  
  Buffer *buffer();
  void set_buffer(Buffer *buffer);
private:
  Buffer2data_writer<value_type>                 buffer2writer;
};

#endif /* SINGLE_DATA_WRITER_CONTROLLER_H */
