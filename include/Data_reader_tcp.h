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
     @param[in] port: the port number on which the reader should
                listen for a connection
   **/
  //Data_reader_tcp(int port = 2630);
//  void open_connection();

  /**
     Constructor.
     @param[in] ip_addr: A list of ip-numbers of the Data_writer to which the reader connects
     @param[in] nAddr: Number of ip-numbers in ip_addr
     @param[in] port: the port number on which the reader should
                listen for a connection
   **/
  Data_reader_tcp(UINT64 *ip_addr, int nAddr, unsigned short int port);
  
  ~Data_reader_tcp();

//  UINT64 move_forward(UINT64 nBytes);
  UINT64 get_bytes(UINT64 nBytes, char *out);

  unsigned int get_port();
  
  bool eof() { return socket < 0; }  
private:
//  std::vector<char> buffer;
  int connection_socket, socket;
  int port;
};

#endif // DATA_READER_TCP_H
