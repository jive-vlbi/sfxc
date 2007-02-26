/*
  $Author$
  $Date$
  $Name$
  $Revision$
  $Source$
*/

#ifndef CONTROLLER_NODE_H
#define CONTROLLER_NODE_H

// #include <runPrms.h>
// #include <genPrms.h>

#include <Node.h>
#include <Controller.h>

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
 * \ingroup Node
 **/
class Manager_node : public Node {
public:
  Manager_node(int numtasks, int rank, char * control_file);
  
  void start();
  
  
  void set_start_time(INT64 start_) { start_time = start_; }
  void set_stop_time(INT64 stop_) { stop_time = stop_; }

  INT64 get_start_time() const { return start_time; }
  INT64 get_stop_time() const { return stop_time; }

  int get_numtasks() { return numtasks; }
  int &get_new_slice_number() { return ++slicenr; }
  
  void add_number_correlator_node(int n) { Ncorrelator_nodes += n; }
  int get_number_correlator_nodes() { return Ncorrelator_nodes; }

  // Callback functions:
  void hook_added_data_reader(int reader);
  void hook_added_data_writer(int writer);
  
private:

  int read_control_file(char *control_file);
  int send_control_parameters_to_controller_node(int rank);

//   RunP RunPrms;
//   GenP GenPrms;
  int numtasks, rank;
  int Nstations, Ncorrelator_nodes;
  int slicenr;
  
  //Controllers:
  Manager_node_controller manager_controller;
  
  INT64 start_time, stop_time;
};

#endif // CONTROLLER_NODE_H
