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
    inline unsigned int bytes_count() {
      return data_.size()-1;
    }
    inline unsigned char* bytes_buffer() {
      return &data_[1];
    }

    inline unsigned int offset() {
      return data_[0];
    }

  private:
    std::vector<unsigned char> data_;
  };



  typedef Memory_pool<Bit_sample_element>                  Bit_sample_memory_pool;
  typedef Bit_sample_memory_pool::Element                  Bit_sample_memory_pool_element;
  typedef Threadsafe_queue<Bit_sample_memory_pool_element> Bit_sample_queue;
  typedef boost::shared_ptr<Bit_sample_queue>              Bit_sample_queue_ptr;
};

#endif // CORRELATOR_NODE_TYPES_H

