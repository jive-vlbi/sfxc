#ifndef INPUT_NODE_TASKLET_H
#define INPUT_NODE_TASKLET_H

#include <boost/shared_ptr.hpp>
#include <queue>

#include "tasklet/tasklet.h"
#include "data_reader.h"
#include "data_writer.h"
#include "delay_table_akima.h"
#include "control_parameters.h"

class Input_node_tasklet : public Tasklet {
public:
  typedef boost::shared_ptr<Data_writer>                   Data_writer_ptr_;

  Input_node_tasklet();
  virtual ~Input_node_tasklet();

  /// set the delay table
  virtual void set_delay_table(Delay_table_akima &delay)=0;

  /// set the track parameters
  virtual void set_parameters(const Input_node_parameters &input_node_param,
                              int node_nr)=0;

  virtual bool has_work() = 0;

  /// Sets a new time interval for which it should output data
  /// (typically the duration of a scan, or part thereof). 
  /// @input Times are in milliseconds
  void add_time_interval(int32_t start_time, int32_t stop_time);

  /// Set the current time interval in the data reader. It is not
  /// possible to go back in time, as the data might be streamed in to
  /// the input node and not be buffered anymore. Time is in milliseconds
  virtual void set_time_interval(int32_t start_time, int32_t stop_time) = 0;


  /// Returns the current time in microseconds
  virtual int get_current_time() = 0;
  /// Returns the stop time of the current time interval in microseconds
  virtual int get_stop_time() = 0;

  /// Sets the output writer for channel i
  virtual void add_data_writer(size_t i,
                               Data_writer_ptr_ data_writer,
                               int nr_seconds) = 0;


  // List of start and stop-times (in seconds) that the reader should output
  std::queue< std::pair<int32_t, int32_t> > time_intervals;

};


/** Returns an input_node_tasklet for the data reader.
 * It determines the number of tracks from the data
 **/
Input_node_tasklet *
get_input_node_tasklet(boost::shared_ptr<Data_reader> reader);

#endif // INPUT_NODE_TASKLET_H
