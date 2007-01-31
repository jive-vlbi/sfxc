/* Author(s): Nico Kruithof, 2007
 * 
 * $Id$
 */

#ifndef LOG_NODE_H
#define LOG_NODE_H

#include <Node.h>
#include <Log_controller.h>
#include <Semaphore_buffer.h>
#include <Ring_buffer.h>

#include <Log_controller.h>
#include <Log_writer_cout.h>

class Log_node : public Node {
public:
  Log_node(int rank, int nNodes);
  ~Log_node();

  void start();

private:
  Log_writer_cout log_writer_cout;

  Log_controller log_controller;
};

#endif // LOG_NODE_H
