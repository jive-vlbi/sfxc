/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: Node.h 251 2007-06-12 13:56:30Z kruithof $
 *
 */

#ifndef ABSTRACT_MANAGER_NODE_H
#define ABSTRACT_MANAGER_NODE_H

#include <Node.h>
#include <Control_parameters.h>
#include <Delay_table_akima.h>
/** Abstract manager node which defines generic functions needed by
    manager nodes.
 **/
class Abstract_manager_node : public Node {
public:
  Abstract_manager_node(int rank, int numtasks, 
                        const Control_parameters &param);
  Abstract_manager_node(int rank, int numtasks,
                        Log_writer *writer, 
                        const Control_parameters &param);
  virtual ~Abstract_manager_node();

  void start_input_node(int rank);
  void start_output_node(int rank);
  void start_correlator_node(int rank);
  void start_log_node(int rank);
  
  void end_node(int rank);

  int get_status(int rank);

  /* set Data_readers */
  // for files
  void set_single_data_reader(int rank, 
                              const std::string &filename);
  void set_multiple_data_reader(int rank, int stream_nr, 
                                const std::string &filename);
  // for tcp
  // ...

  /* set Data_writers */
  // for files
  void set_single_data_writer(int rank, 
                              const std::string &filename);
  void set_multiple_data_writer(int rank, int stream_nr, 
                                const std::string &filename);

  /// Interface to Input node

  // Sets the track parameters for a station:
  void input_node_set(int station, Track_parameters &track_params);
  /// Returns the time in milliseconds since midnight on the start-day
  int32_t input_node_get_current_time(int station);
  void input_node_goto_time(int station, int32_t time);
  void input_node_set_stop_time(int station, int32_t time);
  
  // Send a new time slice, start and stop time are in milliseconds
  void input_node_set_time_slice(int station, int32_t channel, 
                                 int32_t stream_nr, 
                                 int32_t start_time, int32_t stop_time);




  int get_number_of_processes() const;
  const Control_parameters &get_control_parameters() const;

  size_t number_correlator_nodes() const;
  int input_rank(size_t station);
  int correlator_rank(int correlator);
  void correlator_node_set_all(Correlation_parameters &parameters);
  void correlator_node_set_all(Delay_table_akima &delay_table);

protected:
  void wait_for_setting_up_channel(int rank);

  // Data
  Control_parameters control_parameters;
  int numtasks;

  // Contains the MPI_rank for an input_node
  std::vector<int> input_node_rank;
  // Contains the MPI_rank for a correlator_node
  std::vector<int> correlator_node_rank;
};

#endif // ABSTRACT_MANAGER_NODE_H
