#ifndef EAVESDROPPING_TASKLET_H_
#define EAVESDROPPING_TASKLET_H_
#include <iostream>
#include <fstream>
#include "tasklet/tasklet.h"
#include "semaphore_buffer.h"

template <class Buffer>
class Eavesdropping_tasklet : public Tasklet {
public:
  typedef Buffer                            Input_buffer;
  typedef boost::shared_ptr<Input_buffer>   Input_buffer_ptr;
  typedef typename Input_buffer::value_type Input_buffer_element;
  typedef Input_buffer                      Output_buffer;
  typedef Input_buffer_ptr                  Output_buffer_ptr;
  typedef Input_buffer_element              Output_buffer_element;

  Eavesdropping_tasklet(const char * filename) {
    output_buffer_ = Output_buffer_ptr(new Output_buffer());
    out_.open(filename, std::ios::binary);
    assert(out_.is_open());
  }
  ~Eavesdropping_tasklet() {
    out_.close();
  }
  void do_task();

  bool has_work() {
    assert(input_buffer_ != Input_buffer_ptr());
    if (input_buffer_->empty())
      return false;
    //    if (output_buffer_.full())
    //      return false;
    return true;
  }
  const char *name() {
    return "Eavesdropping_tasklet";
  }

  void connect_to(Input_buffer_ptr new_input_buffer) {
    input_buffer_ = new_input_buffer;
  }
  Output_buffer_ptr get_output_buffer() {
    assert(output_buffer_ != Output_buffer_ptr());
    return output_buffer_;
  }

private:
  Input_buffer_ptr  input_buffer_;
  Output_buffer_ptr output_buffer_;
  std::ofstream out_;
};


template <class Buffer>
void
Eavesdropping_tasklet< Buffer >::
do_task() {
  assert(has_work());
#if 1
  Input_buffer_element &input_element = input_buffer_->front();
#if 0 //ELEMENT_IS_VECTOR
  int size = sizeof(typename Input_buffer_element::value_type::value_type)*
             input_element.data().size();
  out_.write((char *)&input_element.data()[0], size);
#elif 0 // Input_node::FFT_BUFFER_ELEMENT
  int size = std::min(input_element.sizeof_fftblock,
                      (int)input_element.data1.data().size() -
                      input_element.sample_offset);
  out_.write((char *)&input_element.data1.data()[input_element.sample_offset],
             size*sizeof(typename Input_buffer_element::value_type));

  if (size < input_element.sizeof_fftblock) {
    out_.write((char *)&input_element.data2.data()[0],
               (input_element.sizeof_fftblock - size) *
               sizeof(typename Input_buffer_element::value_type));
  }
#elif 1 // Input_node::FFT_BUFFER
  int size = sizeof(typename Input_buffer_element::value_type::value_type)*
             input_element.data().data.size();
  out_.write((char *)&input_element.data().data[0], size);

#else
  assert(false);
#endif

  output_buffer_->push(input_element);
  input_buffer_->pop();
#else
  int size;
  Input_buffer_element &input_element = input_buffer_->consume(size);
  Output_buffer_element &output_element = output_buffer_->produce();
  DEBUG_MSG(sizeof(typename Input_buffer_element::value_type::value_type)*size);
  out_.write((char *)&input_element.data()[0],
             sizeof(typename Input_buffer_element::value_type::value_type)*size);
  output_element = input_element;
  input_buffer_->consumed();
  output_buffer_->produced(size);
#endif
}
#endif /*EAVESDROPPING_TASKLET_H_*/
