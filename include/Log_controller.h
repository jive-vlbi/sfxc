/* Author(s): Nico Kruithof, 2007
 * 
 * $Id$
 */

// This include ensures that we read the Log_controller class before the
// Log_node class
#include "Log_node.h"

#ifndef LOG_CONTROLLER_H_
#define LOG_CONTROLLER_H_

#include "Controller.h"
#include "Log_writer.h"

class Log_controller : public Controller
{
public:
	Log_controller(Node &node, int nNodes);

  Process_event_status process_event(MPI_Status &status);

  void set_log_writer(Log_writer *writer);
  bool ready() { return nConnections == 0; }
private:
  Log_writer *log_writer;
  int nConnections;
};

#endif /*LOG_CONTROLLER_H_*/
