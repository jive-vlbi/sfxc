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

#include "controller.h"

#include "data_reader.h"
#include "data_reader2buffer.h"

#include "memory_pool_elements.h"



class Single_data_reader_controller : public Controller {
  typedef Single_data_reader_controller  Self;
public:
  typedef Memory_pool_fixed_size_element<char,256>   data_type;
  typedef Data_reader2buffer<data_type>              Reader2buffer;
  typedef Reader2buffer::value_type                  value_type;
  typedef Reader2buffer::Queue_ptr                   Queue_ptr;

  Single_data_reader_controller(Node &node);

  Process_event_status process_event(MPI_Status &status);

  bool eof();

  Queue_ptr queue();
  void set_queue(Queue_ptr queue);

  std::tr1::shared_ptr<Data_reader> get_data_reader(int i=0);
private:
  void set_data_reader(int stream_nr, std::tr1::shared_ptr<Data_reader> reader);

  Reader2buffer                                      reader2buffer;
};

#endif /* SINGLE_DATA_READER_CONTROLLER_H */
