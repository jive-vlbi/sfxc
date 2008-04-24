#ifndef INPUT_NODE_DATA_WRITER_TASKLET
#define INPUT_NODE_DATA_WRITER_TASKLET

#include "tasklet/tasklet.h"
#include <boost/shared_ptr.hpp>

#include "data_writer.h"
#include "utils.h"
#include "input_node_types.h"
#include "control_parameters.h"

//#ifdef RUNTIME_STATISTIC
//#include "monitor.h"
//#endif // RUNTIME_STATISTIC

class Input_node_data_writer_tasklet : public Tasklet {
public:
  typedef Input_node_types::Fft_buffer       Input_buffer;
  typedef Input_node_types::Fft_buffer_element Input_buffer_element;
  typedef Input_node_types::Fft_buffer_ptr   Input_buffer_ptr;

  typedef boost::shared_ptr<Data_writer>                    Data_writer_ptr;
  struct Writer_struct {
    Data_writer_ptr writer;
    int             slice_size;
  };
  typedef std::queue< Writer_struct >      Data_writer_queue;

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

  //#ifdef RUNTIME_STATISTIC
  //QOS_MonitorSpeed monitor_;
  //#endif // RUNTIME_STATISTIC

};

#endif // INPUT_NODE_DATA_WRITER_TASKLET
