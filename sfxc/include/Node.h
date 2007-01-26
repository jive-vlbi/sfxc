/*
  $Author$
  $Date$
  $Name$
  $Revision$
  $Source$
*/

#ifndef NODE_H
#define NODE_H

#include <types.h>
#include <sfxc_mpi.h>

#include <string>
#include <list>

#include "Controller.h"
#include "Log_writer_mpi.h"

/** Generic node to which a number of controllers can be added.
    \ingroup ImportantClasses
 **/
class Node {
  typedef std::list<Controller *>            Controller_list;
  typedef Controller_list::iterator          Controller_iterator;
public:
  Node(int rank);
  virtual ~Node() {
  }
  /** Start the node
   **/
  void start();

  /** Non-blocking check if a message is available and process it.
   * - -1: terminate node
   * - 0: no message
   * - 1: message processed
   **/
  int check_and_process_waiting_messages();

  /** Blocking check for a message and process it.
   * - false: no message
   * - true: message processed
   **/
  int check_and_process_messages();

  /** Process an MPI event.
      Try to delegate it to the controllers, otherwise produce an error message.
   **/
  int process_event(MPI_Status &status);

  /**
     Produce an error message (either to std::cerr or to a specialised "Log-node")
   **/
  void write_debug(int lvl, const std::string &msg);

  /**
     Add a controller to the node.
   **/
  void add_controller(Controller *controller);
  
  Log_writer &get_log_writer() {
    return log_writer;
  }
  
private:
  int rank;
  Controller_list controllers;

protected:
  Log_writer_mpi log_writer;
};

#endif // NODE_H
