#ifndef LOG_CONTROLLER_H_
#define LOG_CONTROLLER_H_

#include "Controller.h"
#include "Log_writer.h"

class Log_controller : public Controller
{
public:
	Log_controller(Log_writer &writer);

  Process_event_status process_event(MPI_Status &status);
};

#endif /*LOG_CONTROLLER_H_*/
