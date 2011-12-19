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
#include "correlator_time.h"

enum TRANSPORT_TYPE {
  UNINITIALISED = 0,
  MARK5A,
  MARK5B,
  VLBA,
  VDIF
};

class Time_interval {
  public:
    Time_interval(){
    }

    Time_interval(Time &start_time, Time &stop_time){
      start_time_ = start_time;
      stop_time_ = stop_time;
    }

    bool empty(){ return stop_time_ <= start_time_; }

    Time start_time_;
    Time stop_time_;
};

  struct Delay{
    Time time; 
    int32_t  bytes; // the delay in bytes
    int32_t  remaining_samples; // the number of samples to delay after the byte delay
  };
  typedef Memory_pool< std::vector<Delay> >   Delay_memory_pool;
  typedef Delay_memory_pool::Element          Delay_memory_pool_element;

class Input_node_types {
public:
  typedef unsigned char  value_type;
  // Buffer datatype
  struct Memory_pool_data {
    typedef unsigned char      value_type;

    std::vector<value_type> data;
  };
  typedef Memory_pool< Memory_pool_data > Data_memory_pool;
  typedef Data_memory_pool::Element       Data_memory_pool_element;


  // Used to mark a block of data as invalid
  struct Invalid_block{
    // Index of the first byte to be marked invalid
    int invalid_begin;
    // Number of invalid bytes
    int nr_invalid;
  };

  // Memory pool for Mark5 frames
  struct Input_data_frame {
    Input_data_frame()
      : channel(-1), mask(-1) {}
     Data_memory_pool_element  buffer;

    // List of blocks to be flagged as invalid
    std::vector<Invalid_block> invalid;
    // The channel which the data_frame belongs to (currently VDIF only), -1 is broadcast
    int channel;
    // Start time of the mark5 block
    Time start_time;
    // Track mask (flags dead tracks)
    uint64_t mask;
    int seqno;
  };
  /// Buffer for mark5 data frames
  typedef Threadsafe_queue<Input_data_frame>        Input_buffer;
  typedef boost::shared_ptr<Input_buffer>           Input_buffer_ptr;

  /// Buffer for fft buffers
  struct Channel_buffer_element_ {
    Channel_buffer_element_() {}

    Data_memory_pool_element channel_data;
    // List of blocks to be flagged as invalid
    std::vector<Invalid_block> invalid;
    // Time in microseconds
    Time  start_time;
  };
  typedef Channel_buffer_element_                  Channel_buffer_element;
  typedef Threadsafe_queue<Channel_buffer_element> Channel_buffer;
  typedef boost::shared_ptr<Channel_buffer>        Channel_buffer_ptr;

  Input_node_types() {}
};
#endif /*INPUT_NODE_TYPES_H*/
