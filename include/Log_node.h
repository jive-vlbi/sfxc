/* Author(s): Nico Kruithof, 2007
 * 
 * $Id$
 */

#ifndef LOG_NODE_H
#define LOG_NODE_H

#include <Node.h>
#include <Log_writer.h>

class Log_node;

class Log_node_controller : public Controller
{
public:
  Log_node_controller(Node &node, int nNodes);

  Process_event_status process_event(MPI_Status &status);

  void set_log_writer(Log_writer *writer);
  bool ready() { return nConnections == 0; }
private:
  Log_writer *log_writer;
  int nConnections;
};


/**
 * \ingroup Node
 **/
class Log_node : public Node {
public:
  Log_node(int rank, int nNodes);

  void start();

private:
  Log_node_controller log_node_ctrl;
};

#endif // LOG_NODE_H
