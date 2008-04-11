#ifndef OUTPUT_NODE_DATA_READER_TASKLET_H
#define OUTPUT_NODE_DATA_READER_TASKLET_H

#include "tasklet/tasklet.h"
#include <boost/shared_ptr.hpp>

#include "control_parameters.h"
#include "correlator_node_types.h"
#include "data_reader.h"
#include "data_reader_blocking.h"

class Correlator_node_data_reader_tasklet : public Tasklet {
public:
  typedef Correlator_node_types             Types;

  typedef boost::shared_ptr<Data_reader>    Data_reader_ptr;
  typedef boost::shared_ptr<Data_reader_blocking>    Data_reader_blocking_ptr;

  typedef Types::Bit_sample_memory_pool     Output_memory_pool;
  typedef Output_memory_pool::Element       Output_memory_pool_element;  typedef Types::Bit_sample_queue           Output_buffer;  typedef Types::Bit_sample_queue_ptr       Output_buffer_ptr;  Correlator_node_data_reader_tasklet();
  ~Correlator_node_data_reader_tasklet();

  /// Set the input
  void connect_to(Data_reader_ptr reader);
  Output_buffer_ptr get_output_buffer();

  void do_task();
  bool has_work();
  const char *name() {
    return __PRETTY_FUNCTION__;
  }

  void set_parameters(const int n_ffts_to_read,
                      const int bits_per_sample,
                      const int number_channels);

private:
  Data_reader_ptr         reader;
  Data_reader_blocking_ptr    breader_;

  Output_memory_pool output_memory_pool;
  Output_buffer_ptr  output_buffer;

  int n_ffts_to_read;
  int n_bytes_per_fft;
};

#endif // OUTPUT_NODE_DATA_READER_TASKLET_H
