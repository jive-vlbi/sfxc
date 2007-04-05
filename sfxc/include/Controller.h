/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

// Always include Node.h before Controller.h
#include <Node.h>

#ifndef CONTROLLER_H
#define CONTROLLER_H

// Include MPI
#include <types.h>
#include <sfxc_mpi.h>

#include <Log_writer.h>

// Forward declaration
class Node;

/** A controller manages one component of a node, for example the input, 
 * correlation or output. The controller processes MPI events and adjusts the
 * status of the node accordingly. 

    \ingroup ImportantClasses
 **/
class Controller {
public:
  Controller(Node &node);

  virtual ~Controller() {
  }
  
  /// Result of processing an event
  enum Process_event_status {
    PROCESS_EVENT_STATUS_SUCCEEDED = 0,
    PROCESS_EVENT_STATUS_UNKNOWN,
    PROCESS_EVENT_STATUS_FAILED
  };
  
  /**
   * Process an MPI message, if the controller knows how to handle it. 
   **/
  virtual Process_event_status process_event(MPI_Status &status) = 0;
  
  Log_writer &get_log_writer();
  
protected:
  Node &node;
};

#endif // CONTROLLER_H
