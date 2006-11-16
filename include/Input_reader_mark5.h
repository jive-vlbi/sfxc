#ifndef INPUT_READER_MARK5_H
#define INPUT_READER_MARK5_H

#include <Input_reader.h>
#include <vector>
#include <sys/socket.h>

/** Specialisation of Input_reader for reading data from a mark5 disk
    over the network.
 **/
class Input_reader_mark5 : public Input_reader {
public:
  /**
     Constructor.
     @param[in] protocol The network protocol that should be used. For
                now: tcp, udp (not tested). Later: tsunami, streaming
                (tcp for headers, udp for data)?
     @param[in] port: the port number on which the reader should
                listen for a connection
   **/
  Input_reader_mark5(char *protocol = "tcp",
                     int port = 2630);
  ~Input_reader_mark5();

  INT64 move_forward(INT64 nBytes);
  INT64 get_bytes(INT64 nBytes, char *out);
  
private:
  std::vector<char> buffer;
  int sock;
  int msglev;
};

#endif // INPUT_READER_MARK5_H
