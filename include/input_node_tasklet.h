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
#include "integer_delay_correction_per_channel.h"
#include "channel_extractor_tasklet.h"
#include "input_node_data_writer_tasklet.h"

#include "timer.h"

// for RUNTIME_STATISTIC
#include "monitor.h"

enum TRANSPORT_TYPE {
  UNINITIALISED = 0,
  MARK5A,
  MARK5B
};


class Input_node_tasklet : public Tasklet, public Thread {
public:
  typedef boost::shared_ptr<Data_writer>             Data_writer_ptr_;
  typedef Input_data_format_reader                   Input_reader_;
  typedef boost::shared_ptr<Input_reader_>           Input_reader_ptr_;
  typedef Input_data_format_reader_tasklet           Input_reader_tasklet_;
  typedef Channel_extractor_tasklet                  Channel_extractor_tasklet_;
  typedef Integer_delay_correction_per_channel       Integer_delay_tasklet_;
  typedef Input_node_data_writer_tasklet             Data_writer_tasklet_;

  // The mark5a-reader and the first data block
  Input_node_tasklet(Input_reader_ptr_ input_reader_ptr,
                     Input_reader_::Data_frame &data);

  void initialise();

  ~Input_node_tasklet();


  void start_tasklets();
  void stop_tasklets();
  void wait_termination();

  void do_execute();
  void do_task();
  bool has_work();

  const char *name() {
    return __PRETTY_FUNCTION__;
  }

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
  void add_data_writer(size_t i,
                       Data_writer_ptr_ data_writer);

private:
  ThreadPool pool_;

  //  std::list<Time_slice>                time_slices_;
  Input_reader_tasklet_            reader_;
  Channel_extractor_tasklet_       channel_extractor_;

  // Pointer because we can not copy construct the Integer_delay_tasklet_
  // because of the memory pool
  std::vector<Integer_delay_tasklet_ *>  integer_delay_;
  std::vector<Data_writer_tasklet_>    data_writers_;

  bool did_work;

  Timer timer_nothing_;
  Timer timer_delaying_;
  Timer timer_writing_;
  Timer timer_rwriting_;


  Delay_table_akima delay_table;

  // List of start and stop-times (in seconds) that the reader should output
  std::queue< std::pair<int32_t, int32_t> > time_intervals;


#ifdef RUNTIME_STATISTIC
  QOS_MonitorSpeed mark5areader_state_;
  QOS_MonitorSpeed chex_state_;
  QOS_MonitorSpeed integerdelay_state_;
  QOS_MonitorSpeed outputwriter_state_;
  QOS_MonitorSpeed dotask_state_;
#endif //RUNTIME_STATISTIC
  const size_t n_bytes_per_input_word;
};


/** Returns an input_node_tasklet for the data reader.
 * It determines the number of tracks from the data
 **/
Input_node_tasklet *
get_input_node_tasklet(boost::shared_ptr<Data_reader> reader,
                       TRANSPORT_TYPE type);

#endif // INPUT_NODE_TASKLET_H
