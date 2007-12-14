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
  typedef Mk4_memory_pool_element                        Fft_memory_pool_element;
  typedef Memory_pool<std::vector<Type> >                Fft_memory_pool;

  /// Buffer for fft buffers
  struct Fft_buffer_element {
    Fft_memory_pool_element data;
    int                     start_index;
  };
  typedef Semaphore_buffer<Fft_buffer_element>           Fft_buffer;
  typedef boost::shared_ptr<Fft_buffer>                  Fft_buffer_ptr;

  Input_node_types() {}
}
;

#endif /*INPUT_NODE_TYPES_H*/
