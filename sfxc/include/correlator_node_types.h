#ifndef CORRELATOR_NODE_TYPES_H
#define CORRELATOR_NODE_TYPES_H

#include <memory_pool.h>
#include <threadsafe_queue.h>
#include <vector>
#include "sfxc_math.h"
#include "memory_pool_elements.h"
#include "correlator_time.h"

class Correlator_node_types {
public:

  struct Channel_circular_input_buffer { 
    Channel_circular_input_buffer(size_t size_)
      : read(0), write(0), data(size_), size(size_){}
    // NB: We can correlate 36years worth of data @16gb/s per channel before we get
    // integer overflow, therefore we can be sure that read<=write 
    inline size_t bytes_free() {
      return size + read - write;
    }
    inline size_t bytes_read() {
      return write - read;
    }
    typedef unsigned char      value_type;
    std::vector<value_type> data; // The channel extracted from a mark5 frame
    size_t size; // The size of the data buffer
    uint64_t read;  // The index where the next data byte will be read from
    uint64_t write; // The index where the next data byte will be written to
  };
  typedef Channel_circular_input_buffer  *Channel_circular_input_buffer_ptr;

  struct Invalid{
    int start; 
    int n_invalid;
  };
  struct Channel_memory_pool_data {
    Channel_memory_pool_data(): nfft(0) {}
    int nfft;
    Memory_pool_vector_element<FLOAT> data;
  };
  typedef Memory_pool< Channel_memory_pool_data >         Channel_memory_pool;
  typedef Channel_memory_pool::Element                    Channel_memory_pool_element;
  typedef Threadsafe_queue<Channel_memory_pool_element>   Channel_queue;
  typedef boost::shared_ptr<Channel_queue>                Channel_queue_ptr;

  struct Delay_memory_pool_data {
    Delay_memory_pool_data(): stride(0) {}
    // The number of elements reserved for each fft, the start of each fft should be propely (16 bytes) alligned
    size_t stride; 
    Memory_pool_vector_element< std::complex<FLOAT> > data;
  };
  typedef Memory_pool< Delay_memory_pool_data >         Delay_memory_pool;
  typedef Delay_memory_pool::Element                    Delay_memory_pool_element;
  typedef Threadsafe_queue<Delay_memory_pool_element>   Delay_queue;
  typedef boost::shared_ptr<Delay_queue>                Delay_queue_ptr;
};
#endif // CORRELATOR_NODE_TYPES_H
