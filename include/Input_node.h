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
#include <Input_reader.h>

class Input_node : public Node {
public:
  Input_node(int rank);
  ~Input_node();
  void start();

private:
  Input_reader *reader;
};

#endif // INPUT_NODE_H
