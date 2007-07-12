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

// #include <runPrms.h>
// #include <genPrms.h>

#include <Node.h>
#include <Controller.h>
#include <vector>

class Manager_node;

class Manager_node_controller : public Controller
{
public:
  Manager_node_controller(Manager_node &node);

  Process_event_status process_event(MPI_Status &status);
  
  
private:
  Manager_node &node;
};



/**
 * The manager node sends messages to initialize the other nodes and tells them
 * how to connect to each other. After the initialization, it maintains a list 
 * of available correlate nodes and delegates time slices to available 
 * correlate nodes. 
 * 
 * The interface to the user will communicate with the manager node to
 * send commands to the correlator and obtain status information from the
 * correlation process. 
 * 
 * \ingroup Node
 **/
class Manager_node : public Node {
public:
  Manager_node(int numtasks, int rank, char * control_file);
  
  void start();
  
  
  int get_numtasks() { return numtasks; }
  int &get_new_slice_number() { return ++slicenr; }
  
  // Callback functions:
  void hook_added_data_reader(size_t reader);
  void hook_added_data_writer(size_t writer);
  
  /// Different states a correlator node can have
  enum CORRELATING_STATE {
    /// The correlator node is being initialised
    INITIALISING = 0,
    /// The correlator node is currently correlating a time slice
    CORRELATING,
    /// The correlator node is ready to process a time slice
    READY,
    /// The correlator node is terminated
    FINISHED
  };

  /// Set the state of a correlator node  
  void set_correlating_state(int node, CORRELATING_STATE state);
  
private:

  int64_t get_start_time(int input_node_nr);
  void goto_start_time(int input_node_nr, int64_t time);

  int read_control_file(char *control_file);
  int send_control_parameters_to_controller_node(int rank);

  std::vector<CORRELATING_STATE> state_correlate_nodes;
    
//   RunP RunPrms;
//   GenP GenPrms;
  int numtasks, rank;
  int slicenr;
  
  //Controllers:
  Manager_node_controller manager_controller;
  
  int START_INPUT_NODES, START_CORRELATE_NODES, 
      N_INPUT_NODES, N_CORRELATE_NODES;
  
};

#endif // CONTROLLER_NODE_H
