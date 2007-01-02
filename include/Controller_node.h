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
  Controller_node(int numtasks, int rank, char * ctrlFile);
  
  void start();
private:
//   RunP RunPrms;
//   GenP GenPrms;
  int numtasks, rank;
  int Nstations;
};

#endif // CONTROLLER_NODE_H
