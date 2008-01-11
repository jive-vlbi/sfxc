#ifndef INPUT_NODE_DATA_WRITER_TASKLET
#define INPUT_NODE_DATA_WRITER_TASKLET

#include "tasklet/tasklet.h"
#include <boost/shared_ptr.hpp>

#include "data_writer.h"

template <class Type>
class Input_node_data_writer_tasklet : public Tasklet {
public:
  typedef typename Input_node_types<Type>::Channel_buffer         Input_buffer;
  typedef typename Input_node_types<Type>::Channel_buffer_element Input_buffer_element;
  typedef typename Input_node_types<Type>::Channel_buffer_ptr     Input_buffer_ptr;

  typedef boost::shared_ptr<Data_writer>                 Data_writer_ptr;
  typedef std::queue<Data_writer_ptr>                    Data_writer_queue;

  Input_node_data_writer_tasklet();
  virtual ~Input_node_data_writer_tasklet();

  /// Set the input
  void connect_to(Input_buffer_ptr new_input_buffer);

  void add_data_writer(Data_writer_ptr data_writer, int nr_integrations);

  //  /// Get the output
  //  Output_buffer_ptr get_output_buffer();

  void do_task();
  bool has_work();
  const char *name() {
    return __PRETTY_FUNCTION__;
  }

  void set_parameters(Input_node_parameters &input_param);

private:
  Input_buffer_ptr    input_buffer_;
  Data_writer_queue    data_writers_;

  int bytes_per_integration;
};

template <class Type>
Input_node_data_writer_tasklet<Type>::Input_node_data_writer_tasklet() {}

template <class Type>
Input_node_data_writer_tasklet<Type>::~Input_node_data_writer_tasklet() {}

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
         (input_element.data().data.size() <=
          (size_t)data_writers_.front()->get_size_dataslice()));

  data_writers_.front()->put_bytes(input_element.data().data.size(),
                                   &input_element.data().data[0]);

  if (data_writers_.front()->get_size_dataslice() == 0) {
    data_writers_.pop();
  }

  input_element.release();
  input_buffer_->pop();
}

template <class Type>
void
Input_node_data_writer_tasklet<Type>::
add_data_writer(Data_writer_ptr data_writer, int nr_integrations) {
  assert(data_writer->get_size_dataslice() <= 0);
  if (nr_integrations > 0) {
    data_writer->set_size_dataslice(nr_integrations*bytes_per_integration);
  }
  data_writers_.push(data_writer);
}

template <class Type>
void
Input_node_data_writer_tasklet<Type>::
set_parameters(Input_node_parameters &input_param) {
  bytes_per_integration =
    input_param.number_channels*input_param.bits_per_sample()/8;
}


#endif // INPUT_NODE_DATA_WRITER_TASKLET
