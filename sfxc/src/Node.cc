/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <Node.h>
#include <utils.h>
#include <assert.h>

Node::Node(int rank) : rank(rank), log_writer(new Log_writer_mpi(rank, 0)) {
  log_writer->set_mpilevel(1);
}

Node::Node(int rank, Log_writer *writer) : rank(rank), log_writer(writer) {
  log_writer->set_mpilevel(1);
}

Node::~Node() {
  int rank = get_rank();
  MPI_Send(&rank, 1, MPI_INT, 
           RANK_LOG_NODE, MPI_TAG_LOG_MESSAGES_ENDED, MPI_COMM_WORLD);
}

Log_writer &Node::get_log_writer() {
  assert(log_writer != NULL);
  return *log_writer;
}

void 
Node::add_controller(Controller *controller) {
  controllers.push_back(controller);
}

void Node::start() {
  while (true) {
    MESSAGE_RESULT result = check_and_process_message();
    
    switch (result) {
      case MESSAGE_PROCESSED: {
        break;
      }
      case TERMINATE_NODE: {
        return;
      }
      case NO_MESSAGE: {
        assert(false);
        return;
      }
      case ERROR_IN_PROCESSING: {
        get_log_writer().error("Error, failed to process message");
        break;
      }
      case MESSAGE_UNKNOWN: {
        break;
      }
    }
  }
}

Node::MESSAGE_RESULT 
Node::check_and_process_waiting_message() {
    MPI_Status status;
    int result;
    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &result, &status);
    if (result) {
      return process_event(status);
    }
    return NO_MESSAGE;
}

Node::MESSAGE_RESULT 
Node::process_all_waiting_messages() {
  MESSAGE_RESULT result;
  do {
    result = check_and_process_waiting_message();
  } while (result == MESSAGE_PROCESSED);
  return result;
}

Node::MESSAGE_RESULT 
Node::check_and_process_message() {
    MPI_Status status;
    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    MESSAGE_RESULT result = process_event(status); 
    return result;
}

Node::MESSAGE_RESULT 
Node::process_event(MPI_Status &status) {
//  DEBUG_MSG("--- " << get_rank() << ", " << print_MPI_TAG(status.MPI_TAG));
  if (status.MPI_TAG == MPI_TAG_END_NODE) {
    MPI_Status status2; int msg;
    MPI_Recv(&msg, 1, MPI_INT32, status.MPI_SOURCE,
             status.MPI_TAG, MPI_COMM_WORLD, &status2);
    
    get_log_writer().MPI(0, print_MPI_TAG(status.MPI_TAG));
    return TERMINATE_NODE;
  }
  for (Controller_iterator it = controllers.begin();
       it != controllers.end();
       it++) {
    Controller::Process_event_status result = (*it)->process_event(status);
    switch (result) {
    case Controller::PROCESS_EVENT_STATUS_SUCCEEDED: 
      { // Processing succeeded
        return MESSAGE_PROCESSED;
      }
    case Controller::PROCESS_EVENT_STATUS_UNKNOWN: 
      { // Unknown command, try next controller
        continue;
        break;
      }
    case Controller::PROCESS_EVENT_STATUS_FAILED: 
      { // Processing failed
        get_log_writer()(0) 
          << "Error in processing tag:" << status.MPI_TAG << std::endl;
        return ERROR_IN_PROCESSING;
      }
    }
  }
  {
    char msg[80];
    snprintf(msg, 80, "Unknown event %s", print_MPI_TAG(status.MPI_TAG));
    get_log_writer().error(msg);
  }
  
  // Remove event:  
  int size;
  MPI_Get_elements(&status, MPI_CHAR, &size);
  assert(size >= 0);
  char msg[size];
  MPI_Status status2;
  MPI_Recv(&msg, size, MPI_CHAR, status.MPI_SOURCE,
           status.MPI_TAG, MPI_COMM_WORLD, &status2);
  
  return MESSAGE_UNKNOWN;
}
