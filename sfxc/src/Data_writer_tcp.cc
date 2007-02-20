/*
  $Author: kruithof $
  $Date: 2007-01-02 14:39:48 +0100 (Tue, 02 Jan 2007) $
  $Name$
  $Revision: 64 $
  $Source$
*/

#include <Data_writer_tcp.h>
#include <assert.h>

#include <TCP_Connection.h>
#include <iostream>

// defines send:
#include <sys/types.h>
#include <sys/socket.h>

Data_writer_tcp::Data_writer_tcp(int _port) 
: connection_socket(-1), socket(-1), port(_port)
{
  TCP_Connection connection;
  
  connection_socket = connection.open_port(port);
  while (connection_socket <= 0) {
    port ++;
    connection_socket = connection.open_port(port);
  }
}

void Data_writer_tcp::open_connection() {
  TCP_Connection connection;
  socket = connection.open_connection(connection_socket);
  assert(socket > 0);
}

//Data_writer_tcp::Data_writer_tcp(UINT64 ip_addr[], int nAddr, unsigned short int port) 
// : Data_writer(), socket(-1)
//{
//  TCP_Connection connection(true);
//  int i=0;
//  do {
//    socket = connection.do_connect(ip_addr[i], port);
//    i = (i+1)%nAddr;
//  } while (socket <= 0);
//  
//  assert(socket > 0);
//}

Data_writer_tcp::~Data_writer_tcp() {
  close(socket);
}
  
UINT64 
Data_writer_tcp::put_bytes(UINT64 nBytes, char *buff) {
  assert(socket > 0);
  assert(nBytes > 0);
  return write(socket, buff, nBytes);
}


unsigned int Data_writer_tcp::get_port() {
  return port;
}  
