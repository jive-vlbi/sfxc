#ifndef DATA_WRITER_TCP_H
#define DATA_WRITER_TCP_H

#include <Data_writer.h>

#include <vector>

class Data_writer_tcp : public Data_writer {
public:
  Data_writer_tcp(UINT64 *ip_addr, int nAddr, unsigned short int port);
  ~Data_writer_tcp();
  
  UINT64 put_bytes(UINT64 nBytes, char *buff);

private:
  int socket;
};

#endif // DATA_WRITER_TCP_H
