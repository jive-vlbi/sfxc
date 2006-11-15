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
  Input_reader_mark5(char *protocol = "tcp");
  ~Input_reader_mark5();

  INT64 move_forward(INT64 nBytes);
  INT64 get_bytes(INT64 nBytes, char *out);
  
private:
//   /** Reads from the network into the buffer.
//    **/
//   void *read_mark5(void *);

//   sem_t empty, full;
  std::vector<char> buffer;
  int sock;
  int msglev;
};

#endif // INPUT_READER_MARK5_H
