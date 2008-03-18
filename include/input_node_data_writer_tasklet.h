#ifndef INPUT_NODE_DATA_WRITER_TASKLET
#define INPUT_NODE_DATA_WRITER_TASKLET

#include "tasklet/tasklet.h"
#include <boost/shared_ptr.hpp>

#include "data_writer.h"
#include "utils.h"

template <class Type>
class Input_node_data_writer_tasklet : public Tasklet {
public:
  typedef typename Input_node_types<Type>::Fft_buffer         Input_buffer;
  typedef typename Input_node_types<Type>::Fft_buffer_element Input_buffer_element;
  typedef typename Input_node_types<Type>::Fft_buffer_ptr     Input_buffer_ptr;

  typedef boost::shared_ptr<Data_writer>                 Data_writer_ptr;
  typedef std::queue<Data_writer_ptr>                    Data_writer_queue;

  Input_node_data_writer_tasklet();
  virtual ~Input_node_data_writer_tasklet();

  /// Set the input
  void connect_to(Input_buffer_ptr new_input_buffer);

  void add_data_writer(Data_writer_ptr data_writer, int nr_bytes);

  void do_task();
  bool has_work();
  const char *name() {
    return __PRETTY_FUNCTION__;
  }

  void set_parameters(const Input_node_parameters &input_param);

private:
  Input_buffer_ptr    input_buffer_;
  Data_writer_queue    data_writers_;
};


template <class Type>
Input_node_data_writer_tasklet<Type>::
Input_node_data_writer_tasklet() {}

template <class Type>
Input_node_data_writer_tasklet<Type>::~Input_node_data_writer_tasklet() {
  if (input_buffer_ != Input_buffer_ptr()) {
    if (!input_buffer_->empty()) {
      DEBUG_MSG("There is still data to be written. "
                << input_buffer_->size());
    }
  }
  while  (!data_writers_.empty()) {
    if (!data_writers_.front()->get_size_dataslice() == 0) {
      data_writers_.pop();
    } else {
      break;
    }
  }
  if (!data_writers_.empty()) {
    DEBUG_MSG("Data_writers are still waiting to produce output.");
  }
}

template <class Type>
void
Input_node_data_writer_tasklet<Type>::
connect_to(Input_buffer_ptr new_input_buffer) {
  input_buffer_ = new_input_buffer;
}

template <class Type>
bool
Input_node_data_writer_tasklet<Type>::
has_work() {
  if (input_buffer_->empty())
    return false;

  if (data_writers_.empty())
    return false;

  return true;
}

template <class Type>
void
Input_node_data_writer_tasklet<Type>::
do_task() {
  assert(has_work());

  Input_buffer_element &input_element = input_buffer_->front();
  assert((data_writers_.front()->get_size_dataslice() < 0) ||
         (input_element.fft_data.data().data.size() <=
          (size_t)data_writers_.front()->get_size_dataslice()));


  assert(input_element.fft_data.data().data.size() > 0);
  int bytes_to_write = input_element.fft_data.data().data.size();
  int bytes_written = 0;

  do {
    assert(bytes_to_write - bytes_written > 0);
    size_t nbytes =
      data_writers_.front()->put_bytes(bytes_to_write - bytes_written,
                                       (char *)&input_element.fft_data.data().data[bytes_written]);
    assert(nbytes >= 0);
    bytes_written += nbytes;
  } while (bytes_written != bytes_to_write);

  if (data_writers_.front()->get_size_dataslice() == 0) {
    data_writers_.pop();
  }

  input_element.fft_data.release();
  input_buffer_->pop();
}

template <class Type>
void
Input_node_data_writer_tasklet<Type>::
add_data_writer(Data_writer_ptr data_writer, int nr_bytes) {
  assert(data_writer->get_size_dataslice() <= 0);
  if (nr_bytes > 0) {
    data_writer->set_size_dataslice(nr_bytes);
  }
  data_writers_.push(data_writer);
}

template <class Type>
void
Input_node_data_writer_tasklet<Type>::
set_parameters(const Input_node_parameters &input_param) {}


#endif // INPUT_NODE_DATA_WRITER_TASKLET
