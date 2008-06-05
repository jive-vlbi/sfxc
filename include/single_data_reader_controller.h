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
#include "semaphore_buffer.h"
#include "data_reader2buffer.h"



class Single_data_reader_controller : public Controller {
  typedef Single_data_reader_controller  Self;
public:
  typedef Buffer_element_large<char,256>             data_type;
  typedef Data_reader2buffer<data_type>              Reader2buffer;

  typedef Reader2buffer::Memory_pool                 Memory_pool;
  typedef Reader2buffer::value_type                  value_type;
  typedef Reader2buffer::Queue                       Queue;
  typedef Reader2buffer::Queue_ptr                   Queue_ptr;

  Single_data_reader_controller(Node &node);

  Process_event_status process_event(MPI_Status &status);

  bool eof();

  Queue_ptr queue();
  void set_queue(Queue_ptr queue);

  boost::shared_ptr<Data_reader> get_data_reader(int i=0);
private:
  void set_data_reader(int stream_nr, boost::shared_ptr<Data_reader> reader);

  Reader2buffer                                      reader2buffer;
};

#endif /* SINGLE_DATA_READER_CONTROLLER_H */
