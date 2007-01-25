/* Author(s): Nico Kruithof, 2007
 * 
 * $URL:$
 * $Id: $
 */

#include <Output_node.h>

#include <types.h>
#include <Data_writer_file.h>

#include <iostream>
#include <assert.h>

Output_node::Output_node(int rank, int size) 
  : Node(rank), 
    buffer(size), 
    input(buffer, log_writer), output(buffer, log_writer)
{
  log_writer(0)<< "Output_node(rank,size)" << std::endl;
  add_controller(&input);
  add_controller(&output);
}

Output_node::~Output_node() {
  log_writer(1) << "~Output_node()";
}
