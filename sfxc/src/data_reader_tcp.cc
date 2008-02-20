/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <assert.h>
// for inet_ntoa (debugging):
#include <arpa/inet.h>

#include "data_reader_tcp.h"
#include "utils.h"

#include "tcp_connection.h"

Data_reader_tcp::Data_reader_tcp(uint64_t *ip_addr, int nAddr, unsigned short int port)
  : Data_reader(), socket(-1)
{
  TCP_Connection connection(true);
  int i=0;
  do {
    i = (i+1)%nAddr;
    socket = connection.do_connect(ip_addr[i], port);
  } while (socket <= 0);
  
  assert(socket > 0);
}

Data_reader_tcp::~Data_reader_tcp() {
  if (socket > 0) close(socket);
}

int Data_reader_tcp::do_get_bytes(size_t nBytes, char*out) {
  assert(socket > 0);

  if (out == NULL) {
    size_t buff_size = 1000000;
    buff_size = (nBytes < buff_size ? nBytes : buff_size);
    char buff[(int)buff_size];
    return read(socket, (void *) buff, buff_size);
  }
  
  /* Read data from socket */ 
  int result = read(socket, (void *) out, nBytes);
  assert((size_t)result == nBytes);
  return result;
}

unsigned int Data_reader_tcp::get_port() {
  return port;
}
