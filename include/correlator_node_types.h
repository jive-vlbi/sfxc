#ifndef CORRELATOR_NODE_TYPES_H
#define CORRELATOR_NODE_TYPES_H

#include <memory_pool.h>
#include <threadsafe_queue.h>
#include <vector>

class Correlator_node_types {
public:
  struct Bit_sample_element {
    // 2 bits samples
    std::vector<unsigned char> data;
    // sample offset
    char                       offset;
  };

  typedef Memory_pool<Bit_sample_element>      Bit_sample_memory_pool;
  typedef Bit_sample_memory_pool::Element      Bit_sample_memory_pool_element;
  typedef Threadsafe_queue<Bit_sample_memory_pool_element> Bit_sample_queue;
  typedef boost::shared_ptr<Bit_sample_queue>  Bit_sample_queue_ptr;
  

};

#endif // CORRELATOR_NODE_TYPES_H
