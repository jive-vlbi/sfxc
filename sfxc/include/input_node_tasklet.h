#ifndef INPUT_NODE_TASKLET_H
#define INPUT_NODE_TASKLET_H

#include <boost/shared_ptr.hpp>
#include <queue>

#include "tasklet/tasklet.h"
#include "thread.h"

#include "data_reader.h"
#include "data_writer.h"
#include "delay_table_akima.h"
#include "control_parameters.h"

#include "input_data_format_reader_tasklet.h"
#include "channel_extractor_tasklet.h"

#include "input_node_data_writer_tasklet.h"

#include "rttimer.h"

// for RUNTIME_STATISTIC
#include "monitor.h"

enum TRANSPORT_TYPE {
  UNINITIALISED = 0,
  MARK5A,
  MARK5B,
  VLBA
};


class Input_node_tasklet {
public:
  typedef Input_data_format_reader                   Input_reader_;
  typedef boost::shared_ptr<Input_reader_>           Input_reader_ptr_;
  typedef Input_data_format_reader_tasklet           Input_reader_tasklet_;

  // The mark5a-reader and the first data block
  Input_node_tasklet(Input_reader_ptr_ input_reader_ptr,
                     Input_reader_::Data_frame &data);


  ~Input_node_tasklet();

  void initialise();
  void start_tasklets();
  void stop_tasklets();
  void wait_termination();

  /// Sets a new time interval for which it should output data
  /// (typically the duration of a scan, or part thereof).
  /// \param start_time in milliseconds
  /// \param stop_time in milliseconds
  /// It is not possible to go back in time, as the data might be
  /// streamed in to the input node and not be buffered anymore.
  /// Time is in milliseconds
  void add_time_interval(int32_t start_time, int32_t stop_time);

  // Inherited from Input_node_tasklet
  void set_delay_table(Delay_table_akima &delay);
  void set_parameters(const Input_node_parameters &input_node_param,
                      int node_nr);


  /// Returns the current time in microseconds
  int get_current_time();

  /// Returns the stop time of the current time interval in microseconds
  int get_stop_time();

  /// Sets the output writer for channel i
  void add_data_writer(size_t i, Data_writer_sptr data_writer);

  /// Compute a list of delays, note we only store the times(+delay) 
  /// where the integer delay changes
  void get_delays(uint64_t start_time, uint64_t stop_time, 
                               std::vector<Delay> &delay_list);
  /// Calculates the delay at time
  Delay get_delay(int64_t time);
private:
  /// All the thread created in this class are stored in the thread pool
  ThreadPool pool_;

  /// Memory pool to store the delays for each time interval
  Delay_memory_pool delay_pool;

  /// we need one thread for reading
  Input_reader_tasklet_            reader_;

  /// We need one thread for the allocation
  Channel_extractor_tasklet       channel_extractor_;

  /// We need one thread for the writing
  Input_node_data_writer_tasklet   data_writer_;

  Timer rttimer_processing_;
  double last_duration_;

  Delay_table_akima delay_table;

  const size_t n_bytes_per_input_word;

  int delta_time; // the time between two ffts
  int sample_rate;
  int bits_per_sample;
  int64_t size_slice; // Number of samples for one integration slice
};


/** Returns an input_node_tasklet for the data reader.
 * It determines the number of tracks from the data
 **/
Input_node_tasklet *
get_input_node_tasklet(boost::shared_ptr<Data_reader> reader,
                       TRANSPORT_TYPE type, int ref_year, int ref_day);

#endif // INPUT_NODE_TASKLET_H
