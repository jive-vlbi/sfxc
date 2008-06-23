/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#ifndef SINGLE_DATA_WRITER_CONTROLLER_H
#define SINGLE_DATA_WRITER_CONTROLLER_H

#include "controller.h"

#include "data_writer.h"
#include "buffer2data_writer.h"
#include "tcp_connection.h"

#include <memory_pool.h>
#include "memory_pool_elements.h"
#include <threadsafe_queue.h>


class Single_data_writer_controller : public Controller {
  typedef Single_data_writer_controller  Self;
public:
  typedef Memory_pool_vector_element<char>  data_type;
  typedef Memory_pool<data_type>            Memory_pool;
  typedef Memory_pool::value_type           pool_type;
  struct value_type {
    int       actual_size;
    pool_type data;
  };
  typedef Threadsafe_queue<value_type>      Queue;
  typedef boost::shared_ptr<Queue>          Queue_ptr;
  

  Single_data_writer_controller(Node &node);
  ~Single_data_writer_controller();

  Process_event_status process_event(MPI_Status &status);

  Queue_ptr queue();
  void set_queue(Queue_ptr queue);
  boost::shared_ptr<Data_writer> get_data_writer(int i);
private:
  void set_data_writer(int streamnr, boost::shared_ptr<Data_writer> writer);

  Buffer2data_writer<value_type>                 buffer2writer;

  TCP_Connection tcp_connection;
};

#endif /* SINGLE_DATA_WRITER_CONTROLLER_H */
