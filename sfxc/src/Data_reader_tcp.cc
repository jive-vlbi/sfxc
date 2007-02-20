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

UINT64 Data_reader_tcp::get_bytes(UINT64 nBytes, char*out) {
  // NGHK: TODO: check that out != NULL
  assert(socket > 0);
  UINT64 nRead = 0;
  while (nRead < nBytes) {
    /* Read data from socket */ 
    UINT64 size = recv(socket, (void *) out, nBytes-nRead, 0);
    if (size == 0) {
      // Connection closed
      close(socket);
      socket = -1;
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
