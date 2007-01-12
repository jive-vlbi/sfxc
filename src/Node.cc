/*
CVS keywords
$Author$
$Date$
$Name$
$Revision$
$Source$

Author     : NGH Kruithof
StartDate  : 20061101
Last change: 20061124
*/

#include <Node.h>
#include <iostream>
#include <assert.h>

Node::Node(int rank) : rank(rank), debug_level(1) {
}

void 
Node::add_controller(Controller *controller) {
  controllers.push_back(controller);
}

void Node::start() {
  while (true) {
    MPI_Status status;
    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if (status.MPI_TAG == MPI_MSG_CORRELATION_READY) return;
    process_event(status);
  }
}

void Node::process_event(MPI_Status &status) {
  for (Controller_iterator it = controllers.begin();
       it != controllers.end();
       it++) {
    Controller::Process_event_status result = (*it)->process_event(status);
    switch (result) {
    case Controller::PROCESS_EVENT_STATUS_SUCCEEDED: // Processing succeeded
      {
        return;
      }
    case Controller::PROCESS_EVENT_STATUS_UNKNOWN: // Unknown command, try next controller
      {
        continue;
        break;
      }
    case Controller::PROCESS_EVENT_STATUS_FAILED: // Processing failed
      {
        write_debug(1, "Error in processing");
        write_debug(1, status);
        return;
      }
    }
  }
  write_debug(1, "ERROR: Unknown event");
  write_debug(1, status);

  // Remove event:  
  int size;
  MPI_Get_elements(&status, MPI_CHAR, &size);
  assert(size > 0);
  char msg[size];
  MPI_Status status2;
  MPI_Recv(&msg, size, MPI_CHAR, status.MPI_SOURCE,
           status.MPI_TAG, MPI_COMM_WORLD, &status2);
  
}

void Node::write_debug(int lvl, const std::string &msg) {
  if (lvl <= debug_level)
    std::cerr << "Rank " << rank << ": " << msg << std::endl;
}

void Node::write_debug(int lvl, const MPI_Status &status) {
  if (lvl <= debug_level)
    std::cerr << "Rank " << rank << ": "
              << "Source: " << status.MPI_SOURCE 
	      << ", Tag: " << status.MPI_TAG << std::endl;
}

