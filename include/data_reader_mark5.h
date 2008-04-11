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

#include <vector>
#include <sys/socket.h>

#include "data_reader.h"

/** Specialisation of Data_reader for reading data from a mark5 disk
    over the network.
 **/
class Data_reader_mark5 : public Data_reader {
public:
  /**
     Constructor.
     @param[in] urlstr The string describe the protocol and port in
  the following format:
  mark5://protocol:port
  the currently support protocols are:
  mark5://tcp:port
  mark5://udp:port
   **/
  Data_reader_mark5(const std::string& urlstr);

  /**
     Constructor.
     @param[in] protocol The network protocol that should be used. For
                now: tcp, udp (not tested). Later: tsunami, streaming
                (tcp for headers, udp for data)?
     @param[in] port: the port number on which the reader should
                listen for a connection
   **/
  Data_reader_mark5(const char *protocol = "tcp",
                    const int port = 2630);
  ~Data_reader_mark5();


  bool eof();

  bool can_read();

private:
  void initialize(const char *protocol, const int port);

  int do_get_bytes(size_t nBytes, char *out);

  int sock;
  int msglev;
  bool _eof;
};

#endif // DATA_READER_MARK5_H
