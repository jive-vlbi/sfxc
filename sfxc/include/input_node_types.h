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
    Mk4_frame_data() 
      : invalid_bytes_begin(-1), nr_invalid_bytes(-1), start_time(-1) {}
    std::vector<value_type>        mk4_data;

    // The following two members are for the randomization of the mk4-header
    // The number of the first invalid byte
    int invalid_bytes_begin;
    // Number of invalid bytes in this mark4 block
    int nr_invalid_bytes;

    // Start time of the mark4-block in microseconds from midnight
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

    // The channel extracted from a mark4 frame
    std::vector<value_type> data;
  };
  typedef Memory_pool< Channel_memory_pool_data > Channel_memory_pool;
  typedef Channel_memory_pool::Element            Channel_memory_pool_element;

  /// Buffer for fft buffers
  struct Channel_buffer_element_ {
    Channel_buffer_element_()
      : invalid_samples_begin(-1), nr_invalid_samples(-1), start_time(-1) {}

    Channel_memory_pool_element channel_data;
    // The number of the first invalid sample
    int invalid_samples_begin;
    // Number of invalid samples in this fft
    int nr_invalid_samples;
    // Time in microseconds
    int64_t                     start_time;
  };
  typedef Channel_buffer_element_                  Channel_buffer_element;
  typedef Threadsafe_queue<Channel_buffer_element> Channel_buffer;
  typedef boost::shared_ptr<Channel_buffer>        Channel_buffer_ptr;

  /// Buffer for fft buffers
  struct Fft_buffer_element_ {
    Fft_buffer_element_()
      : release_data(false), delay(-1),
        invalid_samples_begin(-1), nr_invalid_samples(-1),
        first_byte(-1), nr_bytes(-1) {}
    // if release_data, the data should not be written, but the data block
    // should be released
    bool release_data;

    // The data
    Channel_memory_pool_element channel_data;

    // If delay >= 0, then we print the header of the fft
    // (otherwise the fft is split over two mark4-fft frames and we are 
    //  processing the second mark4-block);

    // Integer delay of samples within one byte
    char delay;
    // The number of the first invalid sample
    int32_t invalid_samples_begin;
    // Number of invalid samples in this fft
    int32_t nr_invalid_samples;

    // Pointer to the first byte that has to be written, and the number of bytes
    int first_byte, nr_bytes;
  };
  typedef Fft_buffer_element_                      Fft_buffer_element;
  typedef Threadsafe_queue<Fft_buffer_element>     Fft_buffer;
  typedef boost::shared_ptr<Fft_buffer>            Fft_buffer_ptr;

  Input_node_types() {}
}
;

#endif /*INPUT_NODE_TYPES_H*/
