#ifndef OUTPUT_NODE_DATA_READER_TASKLET_H
#define OUTPUT_NODE_DATA_READER_TASKLET_H

#include "tasklet/tasklet.h"
#include <tr1/memory>

#include "data_writer.h"
#include "utils.h"

class Correlator_node_data_reader_tasklet : public Tasklet {
public:
  typedef Correlator_node::Data_reader_ptr           Data_reader_ptr;
  typedef Correlator_node::Bit_sample_buffer_ptr     Output_buffer_ptr;
  typedef Correlator_node::Bit_sample_buffer_element Output_buffer_element;

  Correlator_node_data_reader_tasklet();
  ~Correlator_node_data_reader_tasklet();

  /// Set the input
  void connect_to(Data_reader_ptr reader);
  Output_buffer_ptr get_output_buffer();

  void do_task();
  bool has_work();
  const char *name() {
    return __PRETTY_FUNCTION__;
  }

  void add_parameters(const Input_node_parameters &input_param);

private:
  Data_reader_ptr              reader_ptr;
  Output_buffer_ptr            output_buffer_ptr;
};

#endif // OUTPUT_NODE_DATA_READER_TASKLET_H
