/*
  $Author$
  $Date$
  $Name$
  $Revision$
  $Source$
*/

#ifndef CORRELATOR_NODE_H
#define CORRELATOR_NODE_H

#include <Node.h>

class Correlator_node : public Node {
public:
  Correlator_node(int rank);
  /** Start the correlation node
   **/
  void start();

private:
  /** Notifies an input channel that the node is available for
      correlation using an input connection via tcp.

      @param[in] rank_reader the rank of the input reader to be notified
   **/
  void open_input_channel_tcp(int rank_reader);
};

#endif // CORRELATOR_NODE_H
