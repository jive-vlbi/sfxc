#ifndef CORRELATOR_NODE_TYPES_H
#define CORRELATOR_NODE_TYPES_H

#include <memory_pool.h>
#include <threadsafe_queue.h>
#include <vector>
#include "memory_pool_elements.h"

class Correlator_node_types {
public:
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

  struct Channel_memory_pool_data { // was called Channel_memory_pool_data
    Channel_memory_pool_data()
//      : invalid_samples_begin(-1), nr_invalid_samples(-1)
      : start_time(-1), data(INPUT_NODE_PACKET_SIZE) {}
    typedef unsigned char      value_type;
    // The channel extracted from a mark5 frame
    std::vector<value_type> data;

    // The number of the first invalid sample
    int invalid_samples_begin;
    // Number of invalid samples in this fft
    int nr_invalid_samples;
    // The delay in samples
    int delay;
    // The start time of the data in microseconds
    int64_t start_time;
  };


  typedef Memory_pool< Channel_memory_pool_data >         Channel_memory_pool;
  typedef Channel_memory_pool::Element                    Channel_memory_pool_element;

  typedef Threadsafe_queue<Channel_memory_pool_element>   Channel_buffer;
  typedef boost::shared_ptr<Channel_buffer>               Channel_buffer_ptr;

  typedef Memory_pool_vector_element<FLOAT>                Float_element;
  typedef Memory_pool<Float_element>                       Float_memory_pool;
  typedef Threadsafe_queue<Float_memory_pool::Element>     Float_queue;
  typedef boost::shared_ptr<Float_queue>                   Float_queue_ptr;

  typedef Memory_pool_vector_element< std::complex<FLOAT> >   ComplexFloat_element;
  typedef std::vector<ComplexFloat_element>                   ComplexFloat_buffer;
  typedef Memory_pool<ComplexFloat_buffer>                    ComplexFloat_memory_pool;
  typedef Threadsafe_queue<ComplexFloat_memory_pool::Element> ComplexFloat_queue;
  typedef boost::shared_ptr<ComplexFloat_queue>               ComplexFloat_queue_ptr;
};

#endif // CORRELATOR_NODE_TYPES_H

