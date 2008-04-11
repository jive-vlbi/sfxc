/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id: channel_extractor.h 412 2007-12-05 12:13:20Z kruithof $
 *
 */

#ifndef INPUT_NODE_TYPES_H
#define INPUT_NODE_TYPES_H

#include <vector>
#include <boost/shared_ptr.hpp>
#include <threadsafe_queue.h>

#include "memory_pool.h"

class Input_node_types {
public:
  typedef unsigned char  value_type;
  // Memory pool for Mark4 frames
  struct Mk4_frame_data {
    std::vector<value_type>        mk4_data;
    int64_t                        start_time;
  };
  typedef Memory_pool< Mk4_frame_data >           Mk4_memory_pool;
  typedef Mk4_memory_pool::Element                Mk4_memory_pool_element;

  /// Buffer for mark4 data frames
  typedef Mk4_memory_pool_element                 Mk4_buffer_element;
  typedef Threadsafe_queue<Mk4_buffer_element>    Mk4_buffer;
  typedef boost::shared_ptr<Mk4_buffer>           Mk4_buffer_ptr;

  // Memory pool for dechannelized data
  struct Channel_memory_pool_data {
    typedef unsigned char      value_type;

    std::vector<value_type> data;
    // NGHK: TODO: weights
  };
  typedef Memory_pool< Channel_memory_pool_data > Channel_memory_pool;
  typedef Channel_memory_pool::Element            Channel_memory_pool_element;

  /// Buffer for fft buffers
  struct Channel_buffer_element_ {
    Channel_memory_pool_element channel_data;
    int64_t                     start_time; // Time in microseconds
  };
  typedef Channel_buffer_element_                  Channel_buffer_element;
  typedef Threadsafe_queue<Channel_buffer_element> Channel_buffer;
  typedef boost::shared_ptr<Channel_buffer>        Channel_buffer_ptr;

  /// Buffer for fft buffers
  struct Fft_buffer_element_ {
    Fft_buffer_element_()
        : first_sample(-1), nr_samples(-1), delay(-1), release_data(false) {}
    // first element is offset
    // then nr_channels/samples_per_byte+1 bytes containing data
    Channel_memory_pool_element channel_data;
    int first_sample, nr_samples;
    char delay;
    bool release_data;
  };
  typedef Fft_buffer_element_                      Fft_buffer_element;
  typedef Threadsafe_queue<Fft_buffer_element>     Fft_buffer;
  typedef boost::shared_ptr<Fft_buffer>            Fft_buffer_ptr;

  Input_node_types() {}
}
;

#endif /*INPUT_NODE_TYPES_H*/
