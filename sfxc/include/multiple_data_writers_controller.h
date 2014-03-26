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

#include "controller.h"
#include "tcp_connection.h"
#include "data_writer.h"
#include "buffer2data_writer.h"

#include "memory_pool.h"
#include "memory_pool_elements.h"
#include "threadsafe_queue.h"


class Multiple_data_writers_controller : public Controller {
  typedef Multiple_data_writers_controller  Self;
public:
  typedef boost::shared_ptr<Data_writer>            Data_writer_ptr;

  Multiple_data_writers_controller(Node &node);
  Multiple_data_writers_controller(Node &node, int max_connections);
  ~Multiple_data_writers_controller();

  Process_event_status process_event(MPI_Status &status);

  bool ready();

  Data_writer_ptr get_data_writer(unsigned int i);

  // This is the set of listening IP/port
  void get_listening_ip(std::vector<uint64_t>& ip_port);

private:
  void add_data_writer(unsigned int i, Data_writer_ptr writer);

  std::vector<Data_writer_ptr> data_writers;

  TCP_Connection tcp_connection;
};

#endif /* MULTIPLE_DATA_WRITERS_CONTROLLER_H */
