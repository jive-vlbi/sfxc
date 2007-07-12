/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef LOG_NODE_H
#define LOG_NODE_H

#include <Node.h>
#include <Log_writer.h>
#include <Log_writer_mpi.h>

class Log_node;

class Log_node_controller : public Controller
{
public:
  Log_node_controller(Node &node, int nNodes);

  Process_event_status process_event(MPI_Status &status);

  Log_writer &get_log_writer_output();
  void set_log_writer_output(Log_writer *writer);
  bool ready();
private:
  Log_writer *log_writer_output; // For outputting messages
  int nConnections;
};


/**
 * \ingroup Node
 **/
class Log_node : public Node {
public:
  Log_node(int rank, int nNodes);
  ~Log_node();

  void start();

  // Callback functions:
  void hook_added_data_reader(size_t reader);
  void hook_added_data_writer(size_t writer);

private:
  Log_node_controller log_node_ctrl;
};

#endif // LOG_NODE_H
