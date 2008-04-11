#ifndef CORRELATOR_NODE_TYPES_H
#define CORRELATOR_NODE_TYPES_H

#include <memory_pool.h>
#include <threadsafe_queue.h>
#include <vector>

class Correlator_node_types {
public:

  class Bit_sample_element {
  public:
    Bit_sample_element() {
      data_.resize(2);
    }
    inline int raw_size() {
      return data_.size();
    }
    inline unsigned char* raw_buffer() {
      return &data_[0];
    }

    inline void resize_bytes_buffer(const unsigned int newsize) {
      data_.resize(newsize+1);
    }
