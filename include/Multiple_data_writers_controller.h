/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
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
  
  Multiple_data_writers_controller(Node &node);
  ~Multiple_data_writers_controller();
  
  Process_event_status process_event(MPI_Status &status);
  
  Buffer *buffer(unsigned int i);
  void set_buffer(unsigned int i, Buffer *buff);
  
  Buffer2data_writer<value_type> *operator[](int i);
  
  bool ready();
  
private:
  void add_data_writer(unsigned int i, Data_writer *writer);


  Buffer2writer &get_writer(unsigned int i);

  // These are pointers, because a resize of the vector will 
  // copy construct all the elements and then destroy the old 
  // elements and we can't copy construct the extra threads.
  std::vector< Buffer2writer *>  data_writers;
};

#endif /* MULTIPLE_DATA_WRITERS_CONTROLLER_H */
