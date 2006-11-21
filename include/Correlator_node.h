#ifndef CORRELATOR_NODE_H
#define CORRELATOR_NODE_H

/*
  $Author$
  $Date$
  $Name$
  $Revision$
  $Source$
*/

#include <Node.h>

class Correlator_node : public Node {
public:
  Correlator_node(int rank);
  void start();
};

#endif // CORRELATOR_NODE_H
