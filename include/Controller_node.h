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

class Controller_node : public Node {
public:
  Controller_node(int numtasks, int rank, char * control_file);
  
  void start();
private:

  int read_control_file(char *control_file);
  int send_control_parameters_to_controller_node(int rank);

//   RunP RunPrms;
//   GenP GenPrms;
  int numtasks, rank;
  int Nstations;
};

#endif // CONTROLLER_NODE_H
