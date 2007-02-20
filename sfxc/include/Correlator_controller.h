// This include ensures that we read the Input_controller class before the
// Input_node class
#include "Correlator_node.h"

#ifndef CORRELATOR_CONTROLLER_H
#define CORRELATOR_CONTROLLER_H

#include <Controller.h>
#include <Correlator_controller.h>

// Declare the correlator node:
class Correlator_node;

/**
 * Correlator_controller processes specific signals for the Correlator node.
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

#endif // INPUT_CONTROLLER_H
