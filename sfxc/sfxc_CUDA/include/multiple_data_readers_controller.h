/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#ifndef MULTIPLE_DATA_READERS_CONTROLLER_H
#define MULTIPLE_DATA_READERS_CONTROLLER_H

#include "tcp_connection.h"

#include "controller.h"

#include "data_writer.h"
#include "data_reader2buffer.h"
#include "data_reader_buffer.h"

#include "memory_pool_elements.h"
#include "memory_pool.h"
#include "threadsafe_queue.h"


class Multiple_data_readers_controller : public Controller {
  typedef Multiple_data_readers_controller  Self;
public:
  typedef Memory_pool_fixed_size_element<char,5000>  data_type;
  typedef Data_reader2buffer<data_type>              Reader2buffer;
  typedef boost::shared_ptr<Reader2buffer>        Reader2buffer_ptr;
  typedef Data_reader_buffer<Reader2buffer>          Reader_buffer;
  typedef boost::shared_ptr<Reader_buffer>        Reader_buffer_ptr;

  typedef boost::shared_ptr<Data_reader>          Data_reader_ptr;

  typedef Reader2buffer::Memory_pool                 Memory_pool;
  typedef Reader2buffer::value_type                  value_type;
  typedef Reader2buffer::Queue                       Queue;
  typedef Reader2buffer::Queue_ptr                   Queue_ptr;

  Multiple_data_readers_controller(Node &node);
  ~Multiple_data_readers_controller();

  Process_event_status process_event(MPI_Status &status);

  /** This enables asynchronous io for a certain stream
   **/
  void enable_buffering(unsigned int i);

  Queue_ptr get_queue(unsigned int i);

  /** Returns the data reader, which is either the real reader or the
   *  buffered reader if a buffer is set. **/
  Data_reader_ptr get_data_reader(int i);

  /* Returns true if data_reader(i) != NULL */
  bool initialised(unsigned int i);

  /** The number of input readers **/
  size_t number_of_data_readers();

	void stop();

  // This is the set of listening IP/port
	void get_listening_ip(std::vector<uint64_t>& ip_port);
private:

  void add_data_reader(int i, boost::shared_ptr<Data_reader> reader);

  // These are pointers, because a resize of the vector will
  // copy construct all the elements and then destroy the old
  // elements and we can't copy construct the extra threads.
  struct Reader {
    Reader2buffer_ptr  reader2buffer;
    Reader_buffer_ptr  reader_buffer;
  };

  std::vector<Reader> readers;

  TCP_Connection tcp_connection;
};

#endif /* MULTIPLE_DATA_READERS_CONTROLLER_H */
