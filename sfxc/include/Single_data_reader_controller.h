/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
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
  
  boost::shared_ptr<Buffer> buffer();
  void set_buffer(boost::shared_ptr<Buffer> buffer);

  boost::shared_ptr<Data_reader> get_data_reader();
private:
  void set_data_reader(boost::shared_ptr<Data_reader> reader);
  
  Data_reader2buffer<value_type>                 reader2buffer;
};

#endif /* SINGLE_DATA_READER_CONTROLLER_H */
