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
				start_time_ = 0;
				stop_time_ = 0;
		}

		Time_interval(uint64_t start_time, uint64_t stop_time){
				start_time_ = start_time;
				stop_time_ = stop_time;
		}

		bool empty(){ return stop_time_ <= start_time_; }

		uint64_t start_time_;
		uint64_t stop_time_;
};

  struct Delay{
    uint64_t time; 
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


  // Memory pool for Mark5 frames
  struct Input_data_frame {
    Input_data_frame()
      : invalid_bytes_begin(-1), nr_invalid_bytes(-1), start_time(-1), channel(-1) {}
     Data_memory_pool_element  buffer;

    // The following two members are for the randomization of the mark5-header
    // The number of the first invalid byte
    int invalid_bytes_begin;
    // Number of invalid bytes in this mark5 block
    int nr_invalid_bytes;
    // The channel which the data_frame belongs to (currently VDIF only), -1 is broadcast
    int channel;
    // Start time of the mark5-block in microseconds from midnight
    int64_t start_time;
  };
  /// Buffer for mark5 data frames
  typedef Threadsafe_queue<Input_data_frame>        Input_buffer;
  typedef boost::shared_ptr<Input_buffer>           Input_buffer_ptr;

  /// Buffer for fft buffers
  struct Channel_buffer_element_ {
    Channel_buffer_element_()
      : invalid_samples_begin(-1), nr_invalid_samples(-1), start_time(-1) {}

    Data_memory_pool_element channel_data;
    // The number of the first invalid sample
    int invalid_samples_begin;
    // Number of invalid samples in this fft
    int nr_invalid_samples;
    // Time in microseconds
    int64_t  start_time;
  };
  typedef Channel_buffer_element_                  Channel_buffer_element;
  typedef Threadsafe_queue<Channel_buffer_element> Channel_buffer;
  typedef boost::shared_ptr<Channel_buffer>        Channel_buffer_ptr;

  /// Buffer for fft buffers
  struct Fft_buffer_element_ {
    Fft_buffer_element_()
      : delay(-1),
        invalid_samples_begin(-1), nr_invalid_samples(-1),
        first_byte(-1), nr_bytes(-1) {}
    // The data
    Data_memory_pool_element channel_data;

    // If delay >= 0, then we print the header of the fft
    // (otherwise the fft is split over two mark5-fft frames and we are
    //  processing the second mark5-block);

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
