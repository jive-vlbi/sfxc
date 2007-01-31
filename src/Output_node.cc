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
  : Node(rank), buffer(size),     
    output(buffer, get_log_writer())
{
  add_controller(&output);
}

Output_node::~Output_node() {
  log_writer(1) << "~Output_node()";
}
