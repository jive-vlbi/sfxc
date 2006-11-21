#ifndef INPUT_NODE_H
#define INPUT_NODE_H

/*
  $Author$
  $Date$
  $Name$
  $Revision$
  $Source$
*/

#include <Node.h>

class Input_node : public Node {
public:
  Input_node(int rank);
  void start();
};

#endif // INPUT_NODE_H
