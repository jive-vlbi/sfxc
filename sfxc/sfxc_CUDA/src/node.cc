/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#include "node.h"
#include "utils.h"

Node::Node(int rank)
    : rank(rank), log_writer(new Log_writer_mpi(rank, 0)), assertion_raised(false) {}

Node::Node(int rank, Log_writer *writer)
    : rank(rank), log_writer(writer), assertion_raised(false) {}

Node::~Node() {
  int rank = get_rank();
  if (rank != RANK_LOG_NODE) {
    MPI_Send(&rank, 1, MPI_INT,
             RANK_LOG_NODE, MPI_TAG_LOG_MESSAGES_ENDED, MPI_COMM_WORLD);
  }
}

Log_writer &Node::get_log_writer() {
  SFXC_ASSERT(log_writer != NULL);
  return *log_writer;
}

void
Node::add_controller(Controller *controller) {
  controllers.push_back(controller);
}

void Node::start()
{
	state_ = RUNNING;
  while ( state_ != TERMINATED ) {
    MESSAGE_RESULT result = check_and_process_message();

    switch (result) {
      case MESSAGE_PROCESSED: {
        break;
      }
      case NO_MESSAGE: {
        return;
      }
      case ERROR_IN_PROCESSING: {
        get_log_writer()(0) << "Error, failed to process message" << std::endl;
        break;
      }
      case MESSAGE_UNKNOWN: {
        break;
      }
    }
  }
}

void Node::terminate()
{
	SFXC_ASSERT("PLEASE IMPLEMENT TERMINATING NODE!");
	state_ = TERMINATED;
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
  if (status.MPI_TAG == MPI_TAG_END_NODE) {

    MPI_Status status2;
    int32_t msg;
    MPI_Recv(&msg, 1, MPI_INT32, status.MPI_SOURCE,
             status.MPI_TAG, MPI_COMM_WORLD, &status2);
    assertion_raised = (msg == 1);

		terminate();
    return MESSAGE_PROCESSED;
  } else if (status.MPI_TAG == MPI_TAG_SET_MESSAGELEVEL) {
    MPI_Status status2;
    int32_t msg;
    MPI_Recv(&msg, 1, MPI_INT32, status.MPI_SOURCE,
             status.MPI_TAG, MPI_COMM_WORLD, &status2);

    get_log_writer().set_maxlevel(msg);
    return MESSAGE_PROCESSED;
  }

  for (Controller_iterator it = controllers.begin();
       it != controllers.end();
       it++) {
    Controller::Process_event_status result = (*it)->process_event(status);
    switch (result) {
      case Controller::PROCESS_EVENT_STATUS_SUCCEEDED: { // Processing succeeded
        return MESSAGE_PROCESSED;
      }
      case Controller::PROCESS_EVENT_STATUS_UNKNOWN: { // Unknown command, try next controller
        continue;
        break;
      }
      case Controller::PROCESS_EVENT_STATUS_FAILED: { // Processing failed
        get_log_writer()(0)
        << "Error in processing tag:" << status.MPI_TAG << std::endl;
        return ERROR_IN_PROCESSING;
      }
    }
  }
  {
    char msg[80];
    snprintf(msg, 80, "Unknown event %s", print_MPI_TAG(status.MPI_TAG));
    get_log_writer()(0) << msg << std::endl;
    DEBUG_MSG("Source: " << status.MPI_SOURCE);
    DEBUG_MSG(print_MPI_TAG(status.MPI_TAG));
    SFXC_ASSERT_MSG(false,
                    "Unknown message, exiting.");
  }

  // Remove event:
  int size;
  MPI_Get_elements(&status, MPI_CHAR, &size);
  SFXC_ASSERT(size >= 0);
  char msg[size];
  MPI_Status status2;
  MPI_Recv(&msg, size, MPI_CHAR, status.MPI_SOURCE,
           status.MPI_TAG, MPI_COMM_WORLD, &status2);

  return MESSAGE_UNKNOWN;
}

