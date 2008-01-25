#ifndef INPUT_NODE_TASKLET_H
#define INPUT_NODE_TASKLET_H

#include <boost/shared_ptr.hpp>

#include "tasklet/tasklet.h"
#include "data_reader.h"
#include "data_writer.h"
#include "delay_table_akima.h"
#include "control_parameters.h"

class Input_node_tasklet : public Tasklet {
public:
  typedef boost::shared_ptr<Data_writer>                   Data_writer_ptr_;

//  class Time_slice {
//  public:
//    Time_slice();
//    Time_slice(int start_time, int stop_time, Data_writer *writer);
//
//    int start_time, stop_time;
//    Data_writer *writer;
//  };

  Input_node_tasklet();
  virtual ~Input_node_tasklet();

  /// set the delay table
  virtual void set_delay_table(Delay_table_akima &delay)=0;

  /// set the track parameters
  virtual void set_parameters(const Input_node_parameters &input_node_param,
                              int node_nr)=0;

  virtual bool has_work() = 0;

  /// goes to the specified start time in miliseconds
  /// @return: returns the new time
  virtual int goto_time(int time) = 0;

  /// Returns the current time in miliseconds
  virtual int get_current_time() = 0;
  
  /// sets a specified stop time in miliseconds,
  /// after this time, no more data is sent.
  virtual void set_stop_time(int time) = 0;

  /// Sets the output writer for channel i
  virtual void add_data_writer(size_t i,
                               Data_writer_ptr_ data_writer,
                               int nr_seconds) = 0;

};


/** Returns an input_node_tasklet for the data reader.
 * It determines the number of tracks from the data
 **/
Input_node_tasklet *
get_input_node_tasklet(boost::shared_ptr<Data_reader> reader);

#endif // INPUT_NODE_TASKLET_H
