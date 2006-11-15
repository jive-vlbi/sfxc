#ifndef INPUT_READER_FILE_H
#define INPUT_READER_FILE_H

#include <Input_reader.h>
#include <vector>
#include <fstream>

/** Specialisation of Input_reader for reading files from a linux
    filesystem.
 **/
class Input_reader_file : public Input_reader {
  public:
  /** Constructor, reads from file
   **/
  Input_reader_file(char * filename);

  ~Input_reader_file();

  INT64 move_forward(INT64 nBytes);
  INT64 get_bytes(INT64 nBytes, char *out);
  
private:
  std::ifstream file;
  std::vector<char> buffer;
};

#endif // INPUT_READER_FILE_H
