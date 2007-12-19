#ifndef INPUT_NODE_TASKLET_H
#define INPUT_NODE_TASKLET_H

#include "tasklet/tasklet.h"
#include "data_reader.h"
#include "data_writer.h"
#include "delay_table_akima.h"
#include "control_parameters.h"

class Input_node_tasklet : public Tasklet {
public:
  class Time_slice {
  public:
    Time_slice();
    Time_slice(int start_time, int stop_time, Data_writer *writer);

    int start_time, stop_time;
    Data_writer *writer;
  };

  Input_node_tasklet();
  virtual ~Input_node_tasklet();

  /// set the delay table
  virtual void set_delay_table(Delay_table_akima &delay)=0;

  /// set the track parameters
  virtual void set_parameters(Track_parameters &track_param)=0;


  /// goes to the specified start time in miliseconds
  /// @return: returns the new time
  virtual int goto_time(int time) = 0;

  /// Adds a timeslice to the list of slices
  /// @return: returns whether succeeded
  virtual bool append_time_slice(const Time_slice &time_slice) = 0;
  /// Adds a timeslice to the list of slices
  /// @return: returns whether succeeded
  bool append_time_slice(int start_time,
                         int stop_time,
                         Data_writer *writer);
};

/** Returns an input_node_tasklet for the data reader.
 * It determines the number of tracks from the data
 **/
Input_node_tasklet *
get_input_node_tasklet(Data_reader *reader);

#endif // INPUT_NODE_TASKLET_H
