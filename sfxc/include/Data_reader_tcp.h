/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef DATA_READER_TCP_H
#define DATA_READER_TCP_H

#include <Data_reader.h>
#include <vector>
#include <sys/socket.h>

/** Specialisation of Data_reader for reading data from a tcp disk
    over the network.
 **/
class Data_reader_tcp : public Data_reader {
public:
  /**
     Constructor.
     @param[in] ip_addr: A list of ip-numbers of the Data_writer to which the reader connects
     @param[in] nAddr: Number of ip-numbers in ip_addr
     @param[in] port: the port number on which the reader should
                listen for a connection
   **/
  Data_reader_tcp(uint64_t *ip_addr, int nAddr, unsigned short int port);
  
  ~Data_reader_tcp();

  unsigned int get_port();
  
  bool eof() { return socket < 0; }  
private:
  size_t do_get_bytes(size_t nBytes, char *out);


  int connection_socket, socket;
  int port;
};

#endif // DATA_READER_TCP_H
