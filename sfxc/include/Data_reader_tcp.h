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
  Data_reader_tcp(int port = 2630);
  ~Data_reader_tcp();

  UINT64 move_forward(UINT64 nBytes);
  UINT64 get_bytes(UINT64 nBytes, char *out);
  
private:
  std::vector<char> buffer;
  int sock;
  int msglev;
};

#endif // DATA_READER_TCP_H
