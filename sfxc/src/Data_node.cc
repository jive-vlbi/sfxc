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
    log_writer(0,0),     
    input(buffer, log_writer), output(buffer, log_writer)
{
  log_writer(1)<< "Data_node(rank,size)";
  add_controller(&input);
  add_controller(&output);
}

Data_node::~Data_node() {
  log_writer(1) << "~Data_node()";
}
