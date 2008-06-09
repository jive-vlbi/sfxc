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

#include "controller.h"

#include "data_writer.h"
#include "data_reader2buffer.h"
#include "data_reader_buffer.h"

#include "memory_pool_elements.h"
#include <memory_pool.h>
#include <threadsafe_queue.h>


class Multiple_data_readers_controller : public Controller {
  typedef Multiple_data_readers_controller  Self;
public:
  typedef Memory_pool_fixed_size_element<char,5000>  data_type;
  typedef Data_reader2buffer<data_type>              Reader2buffer;
  typedef Data_reader_buffer<data_type>              Reader_buffer;

  typedef Reader2buffer::Memory_pool                 Memory_pool;
  typedef Reader2buffer::value_type                  value_type;
  typedef Reader2buffer::Queue                       Queue;
  typedef Reader2buffer::Queue_ptr                   Queue_ptr;
  
  Multiple_data_readers_controller(Node &node);
  ~Multiple_data_readers_controller();

  Process_event_status process_event(MPI_Status &status);

  /** This enables asynchronous io for a ceirtain stream
   **/
  void enable_buffering(unsigned int i);

  Queue_ptr get_queue(unsigned int i);

  /** Returns the data reader, which is either the real reader or the
   *  buffered reader if a buffer is set. **/
  boost::shared_ptr<Data_reader> get_data_reader(int i);


  /* Reader2buffer *get_data_reader2buffer(int i); */

  /* Returns true if data_reader(i) != NULL */
  bool initialised(unsigned int i);

  /** The number of input readers **/
  size_t number_of_data_readers();
private:

  void add_data_reader(int i, boost::shared_ptr<Data_reader> reader);

  // NGHK: TODO: Factorize into a single class?
  // These are pointers, because a resize of the vector will
  // copy construct all the elements and then destroy the old
  // elements and we can't copy construct the extra threads.
  std::vector< Reader2buffer * >        data_readers;
  std::vector< boost::shared_ptr<Reader_buffer> >
  buffer_readers;
  std::vector< bool >                   reader_known;
};

#endif /* MULTIPLE_DATA_READERS_CONTROLLER_H */
