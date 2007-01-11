/*
  $Author$
  $Date$
  $Name$
  $Revision$
  $Source$
*/

#ifndef CONTROLLER_H
#define CONTROLLER_H

// Include MPI
#include <types.h>

/** A controller manages one component of a node, for example the input, 
 * correlation or output. The controller processes MPI events and adjusts the
 * status of the node accordingly. 

    \ingroup ImportantClasses
 **/
class Controller {
public:
  Controller();

  virtual ~Controller() {
  }
  /** Try to process an event received by the node.
     \return 
     - 0: processing succeeded
     - 1: unknown event
     - 2: processing failed
   **/
  virtual int process_event(MPI_Status &status) = 0;
};

#endif // CONTROLLER_H
