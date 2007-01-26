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
#include <sfxc_mpi.h>
#include <Log_writer.h>

/** A controller manages one component of a node, for example the input, 
 * correlation or output. The controller processes MPI events and adjusts the
 * status of the node accordingly. 

    \ingroup ImportantClasses
 **/
class Controller {
public:
  Controller(Log_writer &log_writer);

  virtual ~Controller() {
  }
  
  enum Process_event_status {
    PROCESS_EVENT_STATUS_SUCCEEDED = 0,
    PROCESS_EVENT_STATUS_UNKNOWN,
    PROCESS_EVENT_STATUS_FAILED
  };
  
  virtual Process_event_status process_event(MPI_Status &status) = 0;
  protected:
  Log_writer &log_writer;
};

#endif // CONTROLLER_H
