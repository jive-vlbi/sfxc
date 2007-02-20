/* Author(s): Nico Kruithof, 2007
 * 
 * $Id$
 */

#ifndef LOG_NODE_H
#define LOG_NODE_H

#include <Node.h>
#include <Log_controller.h>
#include <Log_writer.h>

/**
 * \ingroup Node
 **/
class Log_node : public Node {
public:
  Log_node(int rank, int nNodes);

  void start();

private:
  Log_controller log_controller;
};

#endif // LOG_NODE_H
