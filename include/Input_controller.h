// This include ensures that we read the Input_controller class before the
// Manager_node class
#include "Input_node.h"

#ifndef INPUT_CONTROLLER_H
#define INPUT_CONTROLLER_H

#include <Controller.h>
#include <Buffer.h>
#include <Data_reader.h>

class Input_node;


class Input_controller : public Controller {
public:
  /// TODO: NGHK: Make this type global?
  typedef Buffer_element<char,131072>      value_type;
  typedef Input_controller  Self;
  
  Input_controller(Input_node &node);

  Process_event_status process_event(MPI_Status &status);
private:
  Input_node &node;
};

#endif /* INPUT_CONTROLLER_H */
