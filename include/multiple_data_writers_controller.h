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
  typedef Buffer_element<char,131072>                data_type;
  typedef Memory_pool<data_type>                     Memory_pool;
  typedef Memory_pool::value_type                    pool_type;
  struct value_type {
    int       actual_size;
    pool_type data;
  };
  typedef Threadsafe_queue<value_type>               Queue;
  typedef boost::shared_ptr<Queue>                   Queue_ptr;

  typedef boost::shared_ptr<Data_writer>   Data_writer_ptr;

  Multiple_data_writers_controller(Node &node, int max_connections);
  ~Multiple_data_writers_controller();

  Process_event_status process_event(MPI_Status &status);

  Queue_ptr queue(unsigned int i);
  void set_queue(unsigned int i, Queue_ptr queue);

  Data_writer_ptr operator[](int i);

  bool ready();

  Data_writer_ptr get_data_writer(size_t i);

  int get_rank_node_reader(int i) {
    assert((0<=i) && (i < (int)data_writers.size()));
    return data_writers[i].rank_node_reader;
  }

  int get_stream_number_reader(int i) {
    assert((0<=i) && (i < (int)data_writers.size()));
    return data_writers[i].stream_number_reader;
  }

private:
  class Output_stream {
  public:
    Output_stream()
        : rank_node_reader(-1), stream_number_reader(-1) {}
    /// The actual output stream
    // These are pointers, because a resize of the vector will
    // copy construct all the elements and then destroy the old
    // elements and we can't copy construct the extra threads.
    Data_writer_ptr data_writer_;

    /** The rank of the node the data is sent to, or -1 if the stream
     *  is connected to a file.
     **/
    int rank_node_reader;

    /** The number of the stream under which the data enters the data reader
     **/
    int stream_number_reader;
  };

  void add_data_writer(unsigned int i, Data_writer_ptr writer,
                       int rank_node_reader, int stream_number_reader);


  std::vector< Output_stream >  data_writers;

  TCP_Connection tcp_connection;
};

#endif /* MULTIPLE_DATA_WRITERS_CONTROLLER_H */
