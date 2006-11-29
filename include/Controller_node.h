/*
CVS keywords
$Author$
$Date$
$Name$
$Revision$
$Source$

Author     : NGH Kruithof
StartDate  : 20061101
Last change: 20061124
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
