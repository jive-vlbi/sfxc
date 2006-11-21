#include <Node.h>
#include <iostream>

Node::Node(int rank) : rank(rank) {
}

void Node::write_debug(const std::string &msg) {
  std::cerr << "Rank " << rank << ": " << msg << std::endl;
}
