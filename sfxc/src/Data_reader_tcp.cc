/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <Data_reader_tcp.h>
#include <assert.h>

#include <TCP_Connection.h>
// for inet_ntoa (debugging):
#include <arpa/inet.h>

//Data_reader_tcp::Data_reader_tcp(int _port) 
//: connection_socket(-1), socket(-1),   port(_port)
//{
//  TCP_Connection connection;
//  
//  connection_socket = connection.open_port(port);
//  while (connection_socket <= 0) {
//    port ++;
//    connection_socket = connection.open_port(port);
//  }
//}

Data_reader_tcp::Data_reader_tcp(UINT64 *ip_addr, int nAddr, unsigned short int port)
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

//void Data_reader_tcp::open_connection() {
//  TCP_Connection connection;
//  socket = connection.open_connection(connection_socket);
//  assert(socket > 0);
//}


Data_reader_tcp::~Data_reader_tcp() {
  if (socket > 0) close(socket);
}

size_t Data_reader_tcp::do_get_bytes(size_t nBytes, char*out) {
  // NGHK: TODO: check that out != NULL
  assert(socket > 0);
  UINT64 nRead = 0;

  if (out == NULL) {
    UINT64 buff_size = 1000000;
    buff_size = (nBytes < buff_size ? nBytes : buff_size);
    char buff[(int)buff_size];
    while (nRead < nBytes) {
      UINT64 read_bytes = (buff_size < (nBytes-nRead) ? buff_size : nBytes-nRead);
      UINT64 size = get_bytes(read_bytes, buff);
      if (size == 0) {
        return nRead;
      }
      nRead += size;
    }
    return nRead;
  }
  
  while (nRead < nBytes) {
    /* Read data from socket */ 
    UINT64 size = read(socket, (void *) out, nBytes-nRead);
    if (size == 0) {
      std::cout << "Data_reader_tcp: SIZE == 0" << std::endl;
      // Connection closed
      return nRead;
    }
    nRead += size;
    out += size;
  }
  assert(nRead == nBytes);
  return nRead;
}

unsigned int Data_reader_tcp::get_port() {
  return port;
}
