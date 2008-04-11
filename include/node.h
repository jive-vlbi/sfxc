/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#ifndef NODE_H
#define NODE_H

#include <string>
#include <list>
#include <iostream>

#include "types.h"
#include "sfxc_mpi.h"

#include "controller.h"
#include "log_writer_mpi.h"

/** Generic node to which a number of controllers can be added.
    \ingroup ImportantClasses
    \ingroup Node
 **/
class Node {
  typedef std::list<Controller *>            Controller_list;
  typedef Controller_list::iterator          Controller_iterator;
public:
  enum MESSAGE_RESULT {
    MESSAGE_PROCESSED = 0,
    TERMINATE_NODE,
    NO_MESSAGE,
    ERROR_IN_PROCESSING,
    MESSAGE_UNKNOWN
  };

  Node(int rank);
  Node(int rank, Log_writer *writer);
  virtual ~Node();
  /** Start the node
   **/
  void start();

  /** Non-blocking check if a message is available and process it.
   * - -1: terminate node
   * - 0: no message
   * - 1: message processed
   **/
  MESSAGE_RESULT check_and_process_waiting_message();

  MESSAGE_RESULT process_all_waiting_messages();

  /** Blocking check for a message and process it.
   * - false: no message
   * - true: message processed
   **/
  MESSAGE_RESULT check_and_process_message();

  /** Process an MPI event.
      Try to delegate it to the controllers, otherwise produce an error message.
   **/
  MESSAGE_RESULT process_event(MPI_Status &status);

  /**
     Produce an error message (either to std::cerr or to a specialised "Log-node")
   **/
  void write_debug(int lvl, const std::string &msg);

  /**
     Add a controller to the node.
   **/
  void add_controller(Controller *controller);

  Log_writer &get_log_writer();

  void set_log_writer(Log_writer *new_writer) {
    log_writer = new_writer;
  }

  int get_rank() {
    return rank;
  }

  /** Callback function after adding a data_reader to the input streams.
   * Reader = 0 for a Single_data_reader_controller
   **/
  virtual void hook_added_data_reader(size_t reader) = 0;
  /** Callback function after adding a data_reader to the input streams.
   * Reader = 0 for a Single_data_reader_controller
   **/
  virtual void hook_added_data_writer(size_t writer) = 0;

private:
  int rank;
  Controller_list controllers;

  Log_writer *log_writer;
};

#endif // NODE_H
