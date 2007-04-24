/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef DATA_READER_MARK5_H
#define DATA_READER_MARK5_H

#include <Data_reader.h>
#include <vector>
#include <sys/socket.h>

/** Specialisation of Data_reader for reading data from a mark5 disk
    over the network.
 **/
class Data_reader_mark5 : public Data_reader {
public:
  /**
     Constructor.
     @param[in] protocol The network protocol that should be used. For
                now: tcp, udp (not tested). Later: tsunami, streaming
                (tcp for headers, udp for data)?
     @param[in] port: the port number on which the reader should
                listen for a connection
   **/
  Data_reader_mark5(char *protocol = "tcp",
                     int port = 2630);
  ~Data_reader_mark5();

  bool eof();  
private:
  size_t do_get_bytes(size_t nBytes, char *out);

  int sock;
  int msglev;
  bool _eof;
};

#endif // DATA_READER_MARK5_H
