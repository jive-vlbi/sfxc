#include <Data_reader_tcp.h>
#include <assert.h>

#include <TCP_Connection.h>

Data_reader_tcp::Data_reader_tcp(int _port) 
: connection_socket(-1), port_socket(-1)
{
  port = _port;
  TCP_Connection connection;
  
  connection_socket = connection.open_port(port);
  while (connection_socket <= 0) {
    port ++;
    connection_socket = connection.open_port(port);
  }
}

void Data_reader_tcp::open_connection() {
  TCP_Connection connection;
  port_socket = connection.open_connection(connection_socket);
  assert(port_socket > 0);
}

Data_reader_tcp::~Data_reader_tcp() {
}

UINT64 Data_reader_tcp::get_bytes(UINT64 nBytes, char*out) {
  UINT64 nRead = 0;
  while (nRead < nBytes) {
    /* Read data from socket */ 
    UINT64 size = recv(port_socket, (void *) out, nBytes-nRead, 0);
    if (size == 0) {
      // Connection closed
      return nRead;
    }
    nRead += size;
    out += size;
  }
  return nRead;
}

unsigned int Data_reader_tcp::get_port() {
  return port;
}
