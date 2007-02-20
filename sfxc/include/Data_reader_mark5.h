/*
CVS keywords
$Author$
$Date$
$Name$
$Revision$
$Source$

Author     : NGH Kruithof
StartDate  : 20061101
Last change: 20061124
*/

#ifndef DATA_READER_MARK5_H
#define DATA_READER_MARK5_H

#include <Data_reader.h>
#include <vector>
#include <sys/socket.h>

/** Specialisation of Data_reader for reading data from a mark5 disk
    over the network.
 **/
class Data_reader_mark5 : public Data_reader {
public:
  /**
     Constructor.
     @param[in] protocol The network protocol that should be used. For
                now: tcp, udp (not tested). Later: tsunami, streaming
                (tcp for headers, udp for data)?
     @param[in] port: the port number on which the reader should
                listen for a connection
   **/
  Data_reader_mark5(char *protocol = "tcp",
                     int port = 2630);
  ~Data_reader_mark5();

  //UINT64 move_forward(UINT64 nBytes);
  UINT64 get_bytes(UINT64 nBytes, char *out);

  bool eof();  
private:
//  std::vector<char> buffer;
  int sock;
  int msglev;
  bool _eof;
};

#endif // DATA_READER_MARK5_H
