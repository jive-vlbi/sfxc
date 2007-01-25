#ifndef MANAGER_CONTROLLER_H_
#define MANAGER_CONTROLLER_H_

#include "Controller.h"

class Manager_controller : public Controller
{
public:
  Manager_controller(Log_writer &writer, int numtasks);

  void set_start_time(INT64 start_) { start = start_; }
  void set_stop_time(INT64 stop_) { stop = stop_; }

  Process_event_status process_event(MPI_Status &status);
  
  
private:
  int numtasks;
  INT64 start, stop;
  int slice;
};

#endif /*MANAGER_CONTROLLER_H_*/
