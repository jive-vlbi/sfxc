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
    int result = (*it)->process_event(status);
    switch (result) {
    case 0: // Processing succeeded
      {
	return;
      }
    case 1: // Unknown command, try next controller
      {
	continue;
        break;
      }
    case 2: // Processing failed
      {
	write_debug(1, "Error in processing");
	write_debug(1, status);
	return;
      }
    }
  }
  write_debug(1, "Unknown event");
  write_debug(1, status);
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

