/*
CVS keywords
$Author$
$Date$
$Name$
$Revision$
$Source$
*/

#include <Data_node.h>

#include <types.h>
#include <Data_reader_file.h>

#include <iostream>
#include <assert.h>

Data_node::Data_node(int rank, int size) 
  : Node(rank), 
    buffer(size), 
    input(buffer), output(buffer)
{
  write_debug(1, "Data_node(rank,size)");
  add_controller(&input);
  add_controller(&output);
}

Data_node::~Data_node() {
  write_debug(1, "~Data_node()");
}
