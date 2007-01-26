// This include ensures that we read the Manager_controller class before the
// Manager_node class
#include "Manager_node.h"

#ifndef MANAGER_CONTROLLER_H_
#define MANAGER_CONTROLLER_H_

#include "Controller.h"
#include <Manager_node.h>

class Manager_node;

class Manager_controller : public Controller
{
public:
  Manager_controller(Manager_node &node);

  Process_event_status process_event(MPI_Status &status);
  
  
private:
  Manager_node &node;
//  int numtasks;
//  INT64 start, stop;
//  int slice;
};

#endif /*MANAGER_CONTROLLER_H_*/
