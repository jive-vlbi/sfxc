#include "Log_controller.h"
#include <assert.h>

Log_controller::Log_controller(Log_writer &writer) : Controller(writer)
{
}

Controller::Process_event_status 
Log_controller::process_event(MPI_Status &status) {
  MPI_Status status2;
  switch (status.MPI_TAG) {
    case MPI_TAG_LOG_MESSAGE:
    {
      int size;
      MPI_Get_elements(&status, MPI_CHAR, &size);
      assert(size > 0);
      char message[size];
      MPI_Recv(&message, size, MPI_CHAR, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);
      
      log_writer(0) << message << std::endl;;
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }

  return PROCESS_EVENT_STATUS_UNKNOWN;
}
