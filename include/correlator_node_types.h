#ifndef CORRELATOR_NODE_TYPES_H
#define CORRELATOR_NODE_TYPES_H

#include <memory_pool.h>
#include <threadsafe_queue.h>
#include <vector>

#include <fftw3.h>

class Correlator_node_types {
public:
  struct Bit_sample_element {
    // 2 bits samples
    std::vector<unsigned char> data;
    // sample offset
    char                       offset;
  };

  class Fourier_element {
  public:
    typedef FLOAT         value_type;

    Fourier_element() : buffer_(NULL), size_(0) {
    }

    ~Fourier_element() {
      if (buffer_ != NULL) {
        fftw_free(buffer_);
      }
    }

    inline int size() {
      return size_;
    }

    void resize(size_t size) {
      if ( size != size_ ) {
        if ( buffer_ == NULL ) {
          // fftw_malloc insure that the allocated data
          // is nicely aligned.
          buffer_ = static_cast<value_type*>
            ( fftw_malloc( sizeof(value_type)*size ) );
          size_ = size;
        } else {
          value_type* oldbuffer = buffer_;
          buffer_ = static_cast<value_type*>
            ( fftw_malloc( sizeof(value_type)*size ) );
          size_t min_size = std::min(size_,size);
          size_ = size;
        
          memcpy(buffer_, oldbuffer, sizeof(value_type)*min_size);

          fftw_free(oldbuffer);
        }
      }
    }


    inline value_type& operator[](int i) {
      assert(i >= 0);
      assert(i < (int)size_ );
      return buffer_[i];
    }

    inline value_type* buffer() {
      return buffer_;
    }
  private:

    // We cannot use a std::vector because we cannot control that
    // the data are properly aligned to be used with fftw_xx or SSE
    value_type* buffer_;
    size_t size_;
  };

  typedef Memory_pool<Bit_sample_element>      Bit_sample_memory_pool;
  typedef Bit_sample_memory_pool::Element      Bit_sample_memory_pool_element;
  typedef Threadsafe_queue<Bit_sample_memory_pool_element> Bit_sample_buffer;
  typedef boost::shared_ptr<Bit_sample_buffer> Bit_sample_buffer_ptr;
  
  typedef Memory_pool<Fourier_element>      Fourier_memory_pool;
  typedef Fourier_memory_pool::Element      Fourier_memory_pool_element;
  typedef Threadsafe_queue<Fourier_memory_pool_element> Fourier_buffer;
  typedef boost::shared_ptr<Fourier_buffer>             Fourier_buffer_ptr;

};

#endif // CORRELATOR_NODE_TYPES_H
