/*
CVS keywords
$Author$
$Date$
$Name$
$Revision$
$Source$

Author     : NGH Kruithof
StartDate  : 20061101
Last change: 20061124
*/

#include <Node.h>
#include <iostream>

Node::Node(int rank) : rank(rank) {
}

void Node::write_debug(const std::string &msg) {
  std::cerr << "Rank " << rank << ": " << msg << std::endl;
}
