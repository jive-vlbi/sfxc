/*
CVS keywords
$Author$
$Date$
$Name$
$Revision$
$Source$
*/

#ifndef INPUT_WRITER_TCP_H
#define INPUT_WRITER_TCP_H

#include <types.h>

/** Virtual class defining the interface for forwarding input.
 **/
class Input_writer_tcp {
  public:
  Input_writer_tcp(int ip_addr[4], int port);
  ~Input_writer_tcp() {}

  /** Write bytes to a tcp stream
   **/
  virtual UINT64 write(UINT64 nBytes, char buffer[]) = 0;
};

#endif // INPUT_WRITER_TCP_H
