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
#include "semaphore_buffer.h"

template <class Type>
class Input_node_types {
public:
  // Memory pool for Mark4 frames
  typedef Memory_pool< std::vector<Type> >               Mk4_memory_pool;
  typedef typename Mk4_memory_pool::Element              Mk4_memory_pool_element;

  /// Buffer for mark4 data frames
  typedef Mk4_memory_pool_element                        Mk4_buffer_element;
  typedef Semaphore_buffer<Mk4_buffer_element>           Mk4_buffer;
  typedef boost::shared_ptr<Mk4_buffer>                  Mk4_buffer_ptr;

  
  // Memory pool for fft's
  typedef Memory_pool<std::vector<Type> >                Fft_memory_pool;
  typedef typename Fft_memory_pool::Element              Fft_memory_pool_element;

  /// Buffer for fft buffers
  struct Fft_buffer_element {
    Type *                  data;
    // Each sample of type Type can contain multiple samples
    // Start at sample with offset "offset_in_samples"
    int                     offset_in_samples;
  };
  typedef Semaphore_buffer<Fft_buffer_element>           Fft_buffer;
  typedef boost::shared_ptr<Fft_buffer>                  Fft_buffer_ptr;

  
  // Memory pool for dechannelized data
  struct Channel_memory_pool_data {
    std::vector<char> data;
    // NGHK: TODO: weights
  };
  typedef Memory_pool< Channel_memory_pool_data >        Channel_memory_pool;
  typedef typename Channel_memory_pool::Element          Channel_memory_pool_element;

  /// Buffer for fft buffers
  typedef Channel_memory_pool_element                    Channel_buffer_element;
  typedef Threadsafe_queue<Channel_buffer_element>       Channel_buffer;
  typedef boost::shared_ptr<Channel_buffer>              Channel_buffer_ptr;

  Input_node_types() {}
}
;

#endif /*INPUT_NODE_TYPES_H*/
