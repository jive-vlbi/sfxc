/* Author(s): Nico Kruithof, 2007
 * 
 * $Id: Single_data_reader_controller.cc 153 2007-02-05 09:12:43Z kruithof $
 */

#ifndef SINGLE_DATA_READER_CONTROLLER_H
#define SINGLE_DATA_READER_CONTROLLER_H

#include <Controller.h>

#include <Data_reader.h>
#include <Semaphore_buffer.h>
#include <Data_reader2buffer.h>



class Single_data_reader_controller : public Controller {
  typedef Single_data_reader_controller  Self;
public:
  /// TODO: NGHK: Make this type global?
  typedef Buffer_element<char,131072>      value_type;
  typedef Buffer<value_type>               Buffer;
  
  Single_data_reader_controller(Node &node);

  Process_event_status process_event(MPI_Status &status);
  
  bool eof();
  
  Buffer *buffer();
  void set_buffer(Buffer *buffer);
private:
  void set_data_reader(Data_reader *reader);
  
  Data_reader2buffer<value_type>                 reader2buffer;
};

#endif /* SINGLE_DATA_READER_CONTROLLER_H */
