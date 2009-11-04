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

  struct Channel_circular_input_buffer { 
    Channel_circular_input_buffer(int size_)
      : read(0), write(0), data(size_), size(size_){}
    // NB: We can correlate 36years worth of data @16gb/s per channel before we get
    // integer overflow, therefore we can be sure that read<=write 
    inline int bytes_free(){
      return size + read - write;
    }
    inline int bytes_read(){
      return write - read;
    }
    typedef unsigned char      value_type;
    std::vector<value_type> data; // The channel extracted from a mark5 frame
    int size; // The size of the data buffer
    uint64_t read;  // The index where the next data byte will be read from
    uint64_t write; // The index where the next data byte will be written to
  };
  typedef Channel_circular_input_buffer  *Channel_circular_input_buffer_ptr;


  struct Channel_memory_pool_data {
    Channel_memory_pool_data(): nfft(0) {}
    int nfft;
    Memory_pool_vector_element<FLOAT> data;
  };

  typedef Memory_pool< Channel_memory_pool_data >         Channel_memory_pool;
  typedef Channel_memory_pool::Element                    Channel_memory_pool_element;

  typedef Threadsafe_queue<Channel_memory_pool_element>   Channel_queue;
  typedef boost::shared_ptr<Channel_queue>                Channel_queue_ptr;

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

