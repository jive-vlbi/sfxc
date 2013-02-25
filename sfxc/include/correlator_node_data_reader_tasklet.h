#ifndef OUTPUT_NODE_DATA_READER_TASKLET_H
#define OUTPUT_NODE_DATA_READER_TASKLET_H

#include "tasklet/tasklet.h"
#include <boost/shared_ptr.hpp>

#include "control_parameters.h"
#include "correlator_node_types.h"
#include "data_reader.h"
#include "data_reader_blocking.h"

// The number of bytes that should be free in the input buffer before we start reading
// note that the absolute minimum would be 3 bytes for n_invalid_bytes or n_data_bytes(int16_t) + header
#define INPUT_BUFFER_MINIMUM_FREE   1000

class Correlator_node_data_reader_tasklet : public Tasklet {
public:
  typedef Correlator_node_types                     Types;

  typedef boost::shared_ptr<Data_reader>            Data_reader_ptr;
  typedef boost::shared_ptr<Data_reader_blocking>   Data_reader_blocking_ptr;

  typedef Types::Channel_circular_input_buffer      Input_buffer;
  typedef Types::Channel_circular_input_buffer_ptr  Input_buffer_ptr;

  Correlator_node_data_reader_tasklet();
  ~Correlator_node_data_reader_tasklet();

  /// Set the input
  void connect_to(int stream_nr_, Data_reader_ptr reader);
  Input_buffer_ptr get_output_buffer();

  void do_task();
  bool has_work();

  bool active();
  int get_fd();

  const char *name() {
    return __PRETTY_FUNCTION__;
  }

  void set_parameters();

private:
  Data_reader_ptr           reader;
  Data_reader_blocking_ptr  breader_;

  Input_buffer              input_buffer;

  bool new_stream_available;
  int stream_nr;

  int state;
  enum {IDLE, PROCESSING_STREAM, RECEIVE_DATA};
  /// Indicatates how many bytes still have to be read from the input stream
  size_t bytes_left;
};

#endif // OUTPUT_NODE_DATA_READER_TASKLET_H
