// This include ensures that we read the Correlator_controller class before the
// Correlator_node class
#include "Correlator_node.h"

#ifndef CORRELATOR_CONTROLLER_H
#define CORRELATOR_CONTROLLER_H

#include <Controller.h>
#include <Correlator_controller.h>

// Declare the correlator node:
class Correlator_node;

/**
 * Correlator_controller processes signals from the Controller_node and
 * forwards them to the correlation
 **/ 
class Correlator_controller : public Controller
{
public:
  Correlator_controller(Correlator_node &node);
  ~Correlator_controller();
  
  Process_event_status process_event(MPI_Status &status);
  
private:
  Correlator_node &node;
};

#endif // CORRELATOR_CONTROLLER_H
