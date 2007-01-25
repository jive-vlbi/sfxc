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

Data_writer_tcp::  Data_writer_tcp(UINT64 ip_addr[], int nAddr, unsigned short int port) : 
  Data_writer(), socket(-1)
{
  TCP_Connection connection(true);
  int i=0;
  do {
    socket = connection.do_connect(ip_addr[i],port);
    i = (i+1)%nAddr;
  } while (socket <= 0);
  assert(socket > 0);
}

Data_writer_tcp::~Data_writer_tcp() {
}
  
UINT64 
Data_writer_tcp::put_bytes(UINT64 nBytes, char *buff) {
  return send(socket, buff, nBytes, 0);
}
