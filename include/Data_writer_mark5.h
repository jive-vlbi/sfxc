#ifndef INPUT_WRITER_MARK5_H
#define INPUT_WRITER_MARK5_H

#include <Data_reader.h>

#include <vector>
#include <sys/socket.h>

/** Specialisation of Input_writer for reading data from a mark5 disk
    over the network.
 **/
class Input_writer_mark5 {
public:
  /**
     Constructor.
     @param[in] reader The input reader from which the data is redirected.
     @param[in] protocol The network protocol that should be used. For
                now: tcp, udp (not tested). Later: tsunami, streaming
                (tcp for headers, udp for data)?
     @param[in] port: the port number on which the writer should
                listen for a connection
   **/
  Input_writer_mark5(Data_reader &reader,
                     char *protocol = "tcp",
                     int port = 2630);
  ~Input_writer_mark5();

private:
  std::vector<char> buffer;
  int sock;
  int msglev;
};

#endif // INPUT_WRITER_MARK5_H
