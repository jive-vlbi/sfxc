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
#include <Log_controller.h>
#include <Manager_controller.h>
#include <Log_writer_cout.h>

class Manager_node : public Node {
public:
  Manager_node(int numtasks, int rank, char * control_file);
  
  void start();
private:

  int read_control_file(char *control_file);
  int send_control_parameters_to_controller_node(int rank);

//   RunP RunPrms;
//   GenP GenPrms;
  int numtasks, rank;
  int Nstations;
  
  Log_writer_cout log_writer_cout;
  Log_controller log_controller;
  Manager_controller manager_controller;
};

#endif // CONTROLLER_NODE_H
