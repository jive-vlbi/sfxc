/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#ifndef CONTROLLER_NODE_H
#define CONTROLLER_NODE_H

#include <vector>

#include "abstract_manager_node.h"
#include "controller.h"
#include "output_header.h"

class Manager_node;

class Manager_node_controller : public Controller {
public:
  Manager_node_controller(Manager_node &node);

  Process_event_status process_event(MPI_Status &status);


private:
  Manager_node &node;
};


class Manager_node : public Abstract_manager_node {
public:
  enum Status {
    /// Initialisation of a new scan
    START_NEW_SCAN=0,
    /// Start the correlation of the time slice
    START_CORRELATION_TIME_SLICE,
    /// Acquiring correlator nodes for the next time slice
    START_CORRELATOR_NODES_FOR_TIME_SLICE,
    /// Everything has been set for the time slice, continue to the next.
    GOTO_NEXT_TIMESLICE,
    /// Stop the correlator, wait for the nodes to finish
    STOP_CORRELATING,
    /// Stop the correlator, wait for the nodes to finish
    WAIT_FOR_OUTPUT_NODE,
    /// Terminate the node
    END_NODE
  };
  /// Different states a correlator node can have
  Manager_node(int rank, int numtasks,
               Log_writer *log_writer,
               const Control_parameters &control_parameters);
  ~Manager_node();

  void start();

  void start_next_timeslice_on_node(int corr_node_nr);

  /// Initialise is called from start() to initialise the correlation process.
  void initialise();

  /// Initialises the processing of a certain scan
  void initialise_scan(const std::string &scan);

  void hook_added_data_reader(size_t reader) {};
  void hook_added_data_writer(size_t writer) {};

  /// Called when the output_node is finished
  void end_correlation();
private:
  std::string get_current_mode() const;

  Manager_node_controller manager_controller;
  Status status;

  /// Start day and year of the experiment
  int32_t start_day, start_year;

  /// Start time of the experiment in miliseconds from midnight on the startday
  int32_t start_time;
  /// Stop time of the experiment in miliseconds from midnight on the startday
  int32_t stop_time;
  /// Stop time of the scan in miliseconds from midnight on the startday
  int32_t stop_time_scan;

  /// The number of the integration slice
  int32_t integration_slice_nr;
  /// Number of the slice for the output node
  int32_t output_slice_nr;

  // A list of all scan names.
  std::list<std::string> scans;

  // The current scan number
  size_t current_scan;

  /// the current channel to correlate by a free correlator node
  size_t current_channel;
  size_t current_correlator_node;

  int n_corr_nodes;
};

#endif // CONTROLLER_NODE_H
