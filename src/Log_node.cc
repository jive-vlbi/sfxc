/* Author(s): Nico Kruithof, 2007
 * 
 * $Id$
 */

#include <Log_node.h>

#include <types.h>
#include <Log_writer.h>

#include <iostream>
#include <assert.h>

Log_node::Log_node(int rank, int nNodes) 
  : Node(rank), log_controller(*this, nNodes)
{
  add_controller(&log_controller);
}

Log_node::~Log_node() {
  log_writer(1) << "~Log_node()";
}

void Log_node::start() {
  while (!log_controller.ready()) {
    //std::cout << "**" << std::endl; 
    check_and_process_message();
    //    Node::start();
  }
}
